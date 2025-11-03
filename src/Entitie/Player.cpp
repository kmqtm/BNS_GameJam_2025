#include "../Core/Config.h"
#include "../Core/Utility.h"
#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Player.h"

#include <Siv3D.hpp>

Player::Player()
	: oxygen_(kMaxOxygen)
{
	SetupAnimations();
	anim_controller_.Play(U"float_idle");
}

void Player::SetupAnimations()
{
	// 集約してアニメーション初期化を分離
	Animation ground_idle_animation;
	ground_idle_animation.texture_asset_names = { U"player_stand" };
	ground_idle_animation.frame_duration_sec = 1.0;
	ground_idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"ground_idle", ground_idle_animation);

	Animation float_idle_animation;
	float_idle_animation.texture_asset_names = { U"player_1" };
	float_idle_animation.frame_duration_sec = 1.0;
	float_idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"float_idle", float_idle_animation);

	Animation walk_animation;
	walk_animation.texture_asset_names = { U"player_walk1", U"player_walk2", U"player_walk3", U"player_walk4", U"player_walk5", U"player_walk6" };
	walk_animation.frame_duration_sec = 0.32;
	walk_animation.is_looping = true;
	anim_controller_.AddAnimation(U"walk", walk_animation);

	Animation float_move_animation;
	float_move_animation.texture_asset_names = { U"player_6", U"player_4", U"player_5" };
	float_move_animation.frame_duration_sec = 0.25;
	float_move_animation.is_looping = true;
	anim_controller_.AddAnimation(U"float_move", float_move_animation);

	Animation swim_animation;
	swim_animation.texture_asset_names = { U"player_2", U"player_3" };
	swim_animation.frame_duration_sec = 0.07;
	swim_animation.is_looping = false;
	anim_controller_.AddAnimation(U"swim", swim_animation);

	Animation dead_animation;
	dead_animation.texture_asset_names = { U"player_dead" };
	dead_animation.frame_duration_sec = 1.0;
	dead_animation.is_looping = true;
	anim_controller_.AddAnimation(U"dead", dead_animation);

	Animation ending_animation;
	ending_animation.texture_asset_names = {}
	; // 大量なので見やすく改行して代入
	ending_animation.texture_asset_names = {
		U"player_end1",U"player_end2",U"player_end3",U"player_end4",U"player_end5",
		U"player_end6",U"player_end7",U"player_end8",U"player_end9",U"player_end10",
		U"player_end11",U"player_end12",U"player_end13",U"player_end14",U"player_end15",
		U"player_end16",U"player_end17",U"player_end18",U"player_end19",U"player_end20",
		U"player_end21"
	};
	ending_animation.frame_duration_sec = 0.4;
	ending_animation.is_looping = false;
	anim_controller_.AddAnimation(U"ending", ending_animation);
}

void Player::Update(const Stage& stage)
{
	just_took_damage_ = false;
	UpdateOxygen();

	// 入力は条件がシンプルなので先にチェック
	if(not is_oxygen_empty_ && not is_in_ending_)
	{
		HandleInput();
	}

	UpdatePhysics(stage);
	UpdateAnimation();

	anim_controller_.Update();

	if(is_invincible_ && invincible_timer_.sF() > kInvincibleDurationSec)
	{
		is_invincible_ = false; // 無敵時間終了
	}

	// 衝突処理は独立関数に切り出し
	HandleCollisions();
}

// 入力処理
void Player::HandleInput()
{
	is_moving_x_ = false;
	if(kInputLeft.pressed())
	{
		velocity_.x = Max(velocity_.x - horizontal_accel_, -horizontal_speed_max_);
		is_moving_x_ = true;
		is_facing_right_ = false;
	}
	else if(kInputRight.pressed())
	{
		velocity_.x = Min(velocity_.x + horizontal_accel_, horizontal_speed_max_);
		is_moving_x_ = true;
		is_facing_right_ = true;
	}

	if(kInputAction1.down())
	{
		OnSwimPressed();
	}
}

void Player::OnSwimPressed()
{
	velocity_.y = swim_power_;
	anim_controller_.Play(U"float_idle");
	anim_controller_.Play(U"swim");

	// swim時に酸素を少し消費
	ModifyOxygen(-kOxygenSwimCost);

	// 効果音は非同期ロード次第で再生されるため、呼び出しは安全
	sound_controller_.Play(U"water_craw", false);
}

// 物理演算と位置更新
void Player::UpdatePhysics(const Stage& stage)
{
	// エンディング中でワープ有効時は中心にLerp
	if(is_in_ending_ && ending_warp_enabled_)
	{
		double dx = ending_target_x_ - pos_.x;
		pos_.x = Math::Lerp(pos_.x, ending_target_x_, ending_warp_lerp_);
		if(std::abs(dx) <= ending_snap_threshold_)
		{
			pos_.x = ending_target_x_;
			ending_warp_enabled_ = false;
			velocity_.x = 0.0;
			is_moving_x_ = false;
		}
	}

	ApplyGravity();
	ApplyFriction();
	MoveX(stage);
	MoveY(stage);
	UpdateColliderPosition();
}

void Player::ApplyGravity()
{
	if(velocity_.y < 0)
	{
		velocity_.y += (gravity_ * rising_gravity_multiplier_);
	}
	else
	{
		velocity_.y += gravity_;
	}
	velocity_.y = Min(velocity_.y, terminal_velocity_y_);
}

void Player::ApplyFriction()
{
	if(not is_moving_x_)
	{
		velocity_.x *= friction_;
		if(std::abs(velocity_.x) < 0.1)
		{
			velocity_.x = 0.0;
		}
	}
}

void Player::MoveX(const Stage& stage)
{
	double next_x = pos_.x + velocity_.x;
	double tile_size = stage.GetTileSize();

	if(velocity_.x > 0)
	{
		double sensor_x = next_x + kPhysicsHalfWidth;
		double sensor_y_top = pos_.y - kPhysicsHalfHeight + 1.0;
		double sensor_y_bot = pos_.y + kPhysicsHalfHeight - 1.0;
		double sensor_y_mid = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y_top) || stage.IsSolid(sensor_x, sensor_y_mid) || stage.IsSolid(sensor_x, sensor_y_bot))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - kPhysicsHalfWidth;
			velocity_.x = 0;
		}
		else
		{
			pos_.x = next_x;
		}
	}
	else if(velocity_.x < 0)
	{
		double sensor_x = next_x - kPhysicsHalfWidth;
		double sensor_y_top = pos_.y - kPhysicsHalfHeight + 1.0;
		double sensor_y_bot = pos_.y + kPhysicsHalfHeight - 1.0;
		double sensor_y_mid = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y_top) || stage.IsSolid(sensor_x, sensor_y_mid) || stage.IsSolid(sensor_x, sensor_y_bot))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + kPhysicsHalfWidth;
			velocity_.x = 0;
		}
		else
		{
			pos_.x = next_x;
		}
	}
}

void Player::MoveY(const Stage& stage)
{
	double next_y = pos_.y + velocity_.y;
	double tile_size = stage.GetTileSize();
	is_grounded_ = false;

	if(velocity_.y > 0)
	{
		double sensor_y = next_y + kPhysicsHalfHeight;
		double sensor_x_left = pos_.x - kPhysicsHalfWidth + 1.0;
		double sensor_x_right = pos_.x + kPhysicsHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) - kPhysicsHalfHeight;
			velocity_.y = 0;
			is_grounded_ = true;
		}
		else
		{
			pos_.y = next_y;
		}
	}
	else if(velocity_.y < 0 && (not just_took_damage_))
	{
		double sensor_y = next_y - kPhysicsHalfHeight;
		double sensor_x_left = pos_.x - kPhysicsHalfWidth + 1.0;
		double sensor_x_right = pos_.x + kPhysicsHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) + tile_size + kPhysicsHalfHeight;
			velocity_.y = 0;
		}
		else
		{
			pos_.y = next_y;
		}
	}
	else if(velocity_.y < 0)
	{
		pos_.y = next_y;
	}
}

void Player::UpdateColliderPosition()
{
	collider.shape = RectF{ Arg::center(pos_), kColliderWidth, kColliderHeight };
}

// アニメーション制御
void Player::UpdateAnimation()
{
	if(is_in_ending_)
	{
		// エンディング開始から3秒経過したらendingアニメーションを再生
		if(ending_timer_.sF() >= kEndingAnimationDelaySec)
		{
			if(not anim_controller_.IsPlaying(U"ending"))
			{
				anim_controller_.Play(U"ending");
			}
		}
		else
		{
			if(not anim_controller_.IsPlaying(U"float_idle"))
			{
				anim_controller_.Play(U"float_idle");
			}
		}
		return;
	}

	if(is_oxygen_empty_)
	{
		if(anim_controller_.IsPlaying(U"dead"))
		{
			return;
		}
		anim_controller_.Play(U"dead");

		return;
	}

	if(anim_controller_.IsPlaying(U"swim"))
	{
		if(velocity_.y > 0)
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"float_move");
			}
			else
			{
				anim_controller_.Play(U"float_idle");
			}
		}
	}
	else
	{
		if(is_grounded_)
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"walk");
			}
			else
			{
				anim_controller_.Play(U"ground_idle");
			}
		}
		else
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"float_move");
			}
			else
			{
				anim_controller_.Play(U"float_idle");
			}
		}
	}
}

void Player::Draw(const Vec2& camera_offset) const
{
	if(is_invincible_)
	{
		if((invincible_timer_.ms() % kBlinkIntervalMs) < kBlinkOnDurationMs)
		{
		}
		else
		{
			return;
		}
	}

	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		// エンディングアニメーション用の特別な描画オフセット
		const Vec2 draw_offset = anim_controller_.IsPlaying(U"ending") ? kEndingDrawOffset : kDrawOffset;
		const Vec2 top_left_pos = pos_ - draw_offset;
		const Vec2 draw_pos = top_left_pos - camera_offset;
		const Vec2 final_draw_pos = s3d::Floor(draw_pos);

		if(is_facing_right_)
		{
			texture_asset->mirrored().draw(final_draw_pos);
		}
		else
		{
			texture_asset->draw(final_draw_pos);
		}
	}
	else
	{
		RectF{ Arg::center(s3d::Floor(pos_ - camera_offset)),32,32 }.drawFrame(2, 0, Palette::Red);
	}
}

Vec2 Player::GetPos() const { return pos_; }

void Player::SetPos(const Vec2& new_pos)
{
	pos_ = new_pos;
	velocity_ = Vec2::Zero();
}

void Player::UpdateOxygen()
{
	if(is_in_ending_)
	{
		return;
	}

	double oxygen_drain = kOxygenDrainPerFrame;
	if(is_moving_x_)
	{
		oxygen_drain += kOxygenHorizontalExtraDrain;
	}

	ModifyOxygen(-oxygen_drain);
}

void Player::ModifyOxygen(double amount)
{
	if(is_oxygen_empty_ || amount == 0.0)
	{
		return;
	}

	oxygen_ += amount;
	oxygen_ = Clamp(oxygen_, 0.0, kMaxOxygen);

	if(oxygen_ == 0.0)
	{
		is_oxygen_empty_ = true;
	}
	else
	{
		is_oxygen_empty_ = false;
	}
}

void Player::RecoverOxygen()
{
	ModifyOxygen(kOxygenRecoveryPerFrame);
}

void Player::TakeDamage()
{
	if(is_invincible_ || is_oxygen_empty_ || is_in_ending_)
	{
		return;
	}

	velocity_.x += (is_facing_right_ ? -2.5 : 2.5);

	ModifyOxygen(-kOxygenDamageAmount);

	just_took_damage_ = true;
	is_invincible_ = true;
	invincible_timer_.restart();

	sound_controller_.Play(U"damage2", false);
}

double Player::GetOxygen() const { return oxygen_; }

double Player::GetMaxOxygen() const { return kMaxOxygen; }

bool Player::IsOxygenEmpty() const { return is_oxygen_empty_; }

void Player::Respawn(const Vec2& spawn_pos)
{
	pos_ = spawn_pos;
	velocity_ = Vec2::Zero();
	oxygen_ = kMaxOxygen;
	is_oxygen_empty_ = false;

	is_in_ending_ = false;

	is_invincible_ = true;
	invincible_timer_.restart();

	anim_controller_.Play(U"float_idle");
}

void Player::StartEnding(double camera_center_world_x)
{
	is_in_ending_ = true;
	ending_target_x_ = camera_center_world_x + 80.0;
	ending_warp_enabled_ = true;

	velocity_.y = 0.0;
	velocity_.x = 0.0;

	ending_timer_.restart();
}

void Player::HandleCollisions()
{
	if((not is_invincible_) && (not is_oxygen_empty_) && (not is_in_ending_) && collider.is_colliding)
	{
		for(const auto& tag : collider.collided_tags)
		{
			if(tag == ColliderTag::kEnemy)
			{
				TakeDamage();
				break;
			}
		}
	}
}

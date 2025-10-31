#include "../Core/Config.h"
#include "../Core/Utility.h"
#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Player.h"

#include <Siv3D.hpp>

Player::Player()
	: oxygen_(kMaxOxygen)
{
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
	walk_animation.texture_asset_names = { U"player_walk1", U"player_walk2", U"player_walk3", U"player_walk4", U"player_walk5", U"player_walk6", };
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
}

void Player::Update(const Stage& stage)
{
	just_took_damage_ = false;
	UpdateOxygen();

	// 酸素が0なら入力処理を受け付けない
	if(not is_oxygen_empty_)
	{
		HandleInput();
	}

	UpdatePhysics(stage);
	UpdateAnimation();

	anim_controller_.Update();

	if(is_invincible_)
	{
		if(invincible_timer_.sF() > kInvincibleDurationSec)
		{
			is_invincible_ = false; // 無敵時間終了
		}
	}

	if((not is_invincible_) && (not is_oxygen_empty_) && collider.is_colliding)
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
	else
	{
		velocity_.x *= friction_;
		if(std::abs(velocity_.x) < 0.1)
		{
			velocity_.x = 0.0;
		}
	}

	if(kInputAction1.down())
	{
		velocity_.y = swim_power_;
		anim_controller_.Play(U"float_idle");
		anim_controller_.Play(U"swim");

		// swim時に酸素を少し消費
		ModifyOxygen(-kOxygenSwimCost);
	}
}

// 物理演算と位置更新
void Player::UpdatePhysics(const Stage& stage)
{
	ApplyGravity();
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

void Player::MoveX(const Stage& stage)
{
	const double tile_size = stage.GetTileSize();
	double next_x = pos_.x + velocity_.x;

	if(velocity_.x > 0) // 右へ移動
	{
		const double sensor_x = next_x + kPhysicsHalfWidth;
		const double sensor_y_top = pos_.y - kPhysicsHalfHeight + 1.0;
		const double sensor_y_bot = pos_.y + kPhysicsHalfHeight - 1.0;

		const double sensor_y_mid = pos_.y;

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
	else if(velocity_.x < 0) // 左へ移動
	{
		const double sensor_x = next_x - kPhysicsHalfWidth;
		const double sensor_y_top = pos_.y - kPhysicsHalfHeight + 1.0;
		const double sensor_y_bot = pos_.y + kPhysicsHalfHeight - 1.0;

		const double sensor_y_mid = pos_.y;

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
	const double tile_size = stage.GetTileSize();
	double next_y = pos_.y + velocity_.y;
	is_grounded_ = false;

	if(velocity_.y > 0)
	{
		// センサー: 下面の左端と右端
		const double sensor_y = next_y + kPhysicsHalfHeight;
		const double sensor_x_left = pos_.x - kPhysicsHalfWidth + 1.0;
		const double sensor_x_right = pos_.x + kPhysicsHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			// 衝突，床タイルの上端にスナップ
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) - kPhysicsHalfHeight;
			velocity_.y = 0; // 速度リセット
			is_grounded_ = true; // 着地
		}
		else
		{
			pos_.y = next_y;
		}
	}
	else if(velocity_.y < 0 && (not just_took_damage_))
	{
		// センサー: 上面の左端と右端
		const double sensor_y = next_y - kPhysicsHalfHeight;
		const double sensor_x_left = pos_.x - kPhysicsHalfWidth + 1.0;
		const double sensor_x_right = pos_.x + kPhysicsHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			// 衝突，天井タイルの下端にスナップ
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) + tile_size + kPhysicsHalfHeight;
			velocity_.y = 0; // 速度リセット
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
	// 動的Collider(vs 敵用)の位置を最終座標で更新
	collider.shape = RectF{ Arg::center(pos_), kColliderWidth, kColliderHeight };
}

// アニメーション制御
void Player::UpdateAnimation()
{
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
		const Vec2 top_left_pos = pos_ - kDrawOffset;

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
		RectF{ Arg::center(s3d::Floor(pos_ - camera_offset)), 32, 32 }.drawFrame(2, 0, Palette::Red);
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
	// 基本の減少
	double oxygen_drain = kOxygenDrainPerFrame;

	// 水平移動中は追加で消費
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

	// 0になった瞬間
	if(oxygen_ == 0.0)
	{
		is_oxygen_empty_ = true;
	}
}

void Player::TakeDamage()
{
	if(is_invincible_ || is_oxygen_empty_)
	{
		return;
	}

	velocity_.y = -1.0;

	ModifyOxygen(-kOxygenDamageAmount);

	just_took_damage_ = true;
	is_invincible_ = true;
	invincible_timer_.restart();
}

double Player::GetOxygen() const
{
	return oxygen_;
}

bool Player::IsOxygenEmpty() const
{
	return is_oxygen_empty_;
}

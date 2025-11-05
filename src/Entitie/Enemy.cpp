#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Enemy.h"
#include "Player.h"

#include <Siv3D.hpp>
#include <variant>

Enemy::Enemy(const String& type, const Vec2& center_pos)
	: pos_(center_pos)
	, start_pos_(center_pos)
{
	SetupProperties(type);
	SetupAnimations(type);

	if(type == U"Coral_L" || type == U"Coral_R")
	{
		// Coralは Circle で初期化
		collider_ = Collider{
			Circle{ pos_, kCoralColliderRadius }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"Fish")
	{
		// Fishは RectF で初期化
		collider_ = Collider{
			RectF{ Arg::center(pos_), kFishColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"Clione")
	{
		// Clioneは RectF で初期化
		collider_ = Collider{
			RectF{ Arg::center(pos_), kClioneColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"Shark")
	{
		collider_ = Collider{
			RectF{ Arg::center(pos_), kSharkColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"DeepseaFish")
	{
		collider_ = Collider{
			RectF{ Arg::center(pos_), kDeepseaFishColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"Swimmie")
	{
		collider_ = Collider{
			RectF{ Arg::center(pos_), kSwimmieColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"MorayEel_L" || type == U"MorayEel_R")
	{
		collider_ = Collider{
			RectF{ Arg::center(pos_), kMorayEelColliderSize }, //
			ColliderTag::kEnemy
		};
	}
	else if(type == U"Octoleg_L" || type == U"Octoleg_R")
	{
		collider_ = Collider{
			RectF{ Arg::center(pos_), kOctolegColliderSize }, //
			ColliderTag::kEnemy
		};
	}
}

void Enemy::SetupProperties(const String& type)
{
	if(type == U"Fish")
	{
		behavior_ = EnemyBehavior::Patrol;
		physics_size_ = kFishPhysicsSize;
		velocity_.x = -kFishSpeed;
		is_facing_right_ = false;
		collision_offset_ = 0.0;
	}
	else if(type == U"Shark")
	{
		behavior_ = EnemyBehavior::Patrol;
		physics_size_ = kSharkPhysicsSize;
		velocity_.x = -kSharkSpeed;
		is_facing_right_ = false;
		collision_offset_ = 150.0;
	}
	else if(type == U"DeepseaFish")
	{
		behavior_ = EnemyBehavior::Patrol;
		physics_size_ = kDeepseaFishPhysicsSize;
		velocity_.x = -kDeepseaFishSpeed;
		is_facing_right_ = false;
		collision_offset_ = 0.0;
	}
	else if(type == U"Swimmie")
	{
		behavior_ = EnemyBehavior::Patrol;
		physics_size_ = kSwimmiePhysicsSize;
		velocity_.x = -kSwimmieSpeed;
		is_facing_right_ = false;
		collision_offset_ = 35.0;
	}
	else if(type == U"MorayEel_L")
	{
		behavior_ = EnemyBehavior::BackAndForth;
		physics_size_ = kMorayEelPhysicsSize;
		velocity_.x = -kMorayEelSpeed;
		max_travel_distance_ = kBackAndForthDistance;
	}
	else if(type == U"MorayEel_R")
	{
		behavior_ = EnemyBehavior::BackAndForth;
		physics_size_ = kMorayEelPhysicsSize;
		velocity_.x = kMorayEelSpeed;
		max_travel_distance_ = kBackAndForthDistance;
	}
	else if(type == U"Octoleg_L")
	{
		behavior_ = EnemyBehavior::BackAndForth;
		physics_size_ = kOctolegPhysicsSize;
		velocity_.x = -kOctolegSpeed;
		max_travel_distance_ = kBackAndForthDistance;
	}
	else if(type == U"Octoleg_R")
	{
		behavior_ = EnemyBehavior::BackAndForth;
		physics_size_ = kOctolegPhysicsSize;
		velocity_.x = kOctolegSpeed;
		max_travel_distance_ = kBackAndForthDistance;
	}
	else
	{
		behavior_ = EnemyBehavior::Stationary;
		velocity_ = Vec2::Zero();

		if(type == U"Coral_L")
		{
			physics_size_ = kCoralPhysicsSize;
		}
		else if(type == U"Coral_R")
		{
			physics_size_ = kCoralPhysicsSize;
		}
		else if(type == U"Clione")
		{
			physics_size_ = kClionePhysicsSize;
		}
	}
}

void Enemy::SetupAnimations(const String& type)
{
	Animation anim;

	if(type == U"Fish")
	{
		anim.texture_asset_names = { U"fishA_1", U"fishA_2" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"Shark")
	{
		anim.texture_asset_names = { U"shark" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"DeepseaFish")
	{
		anim.texture_asset_names = { U"deapsea-fishA1", U"deapsea-fishA2" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"Swimmie")
	{
		anim.texture_asset_names = { U"swimmie1", U"swimmie2" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"MorayEel_L")
	{
		anim.texture_asset_names = { U"moray_eel1_l", U"moray_eel2_l", U"moray_eel3_l", U"moray_eel4_l" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"MorayEel_R")
	{
		anim.texture_asset_names = { U"moray_eel1_r", U"moray_eel2_r", U"moray_eel3_r", U"moray_eel4_r" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"Octoleg_L")
	{
		anim.texture_asset_names = { U"octoleg1_l", U"octoleg2_l" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else if(type == U"Octoleg_R")
	{
		anim.texture_asset_names = { U"octoleg1_r", U"octoleg2_r" };
		anim.frame_duration_sec = 0.5;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	else // Stationary型
	{
		anim.is_looping = true;

		if(type == U"Coral_L")
		{
			anim.texture_asset_names = { U"coral_l" };
			anim.is_looping = false;
		}
		else if(type == U"Coral_R")
		{
			anim.texture_asset_names = { U"coral_r" };
			anim.is_looping = false;
		}
		else if(type == U"Clione")
		{
			anim.texture_asset_names = { U"clione1", U"clione2", U"clione3" };
			anim.frame_duration_sec = 0.5;
		}
		else
		{
			Print << U"エラー: 未知の敵タイプ '{}' です．"_fmt(type);
			anim.texture_asset_names = { U"coral_l" }; // フォールバック
			anim.is_looping = false;
		}

		anim_controller_.AddAnimation(U"idle", anim);
		anim_controller_.Play(U"idle");
	}
}

void Enemy::Update(const Stage& stage, const Player& player)
{
	if(not is_alive_) return;

	UpdateAI(stage);
	anim_controller_.Update();
	UpdateColliderPosition();
	HandleCollision();
}

void Enemy::UpdateAI(const Stage& stage)
{
	// 振る舞いに応じてロジックを更新
	if(behavior_ == EnemyBehavior::Patrol)
	{
		UpdatePatrol(stage);
	}
	else if(behavior_ == EnemyBehavior::BackAndForth)
	{
		UpdateBackAndForth(stage);
	}
	// Stationaryの場合は何もしない
}

void Enemy::UpdateColliderPosition()
{
	std::visit([&](auto& shape)
			   {
				   if constexpr(std::is_same_v<std::decay_t<decltype(shape)>, s3d::Circle> ||
								std::is_same_v<std::decay_t<decltype(shape)>, s3d::RectF>)
				   {
					   shape.setCenter(pos_);
				   }
			   }, collider_.shape);
}

void Enemy::HandleCollision()
{
	// 本ゲームでは特に敵側で行う処理なし
}

// 巡回ロジック
void Enemy::UpdatePatrol(const Stage& stage)
{
	const double next_x = pos_.x + velocity_.x;
	const double half_width = physics_size_.x / 2.0;
	const double tile_size = stage.GetTileSize();

	if(velocity_.x > 0) // 右に移動中
	{
		// collision_offset_を加えて早めに検知
		const double sensor_x = next_x + half_width + collision_offset_;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - half_width - collision_offset_;
			velocity_.x *= -1.0;
			is_facing_right_ = false;
		}
		else
		{
			pos_.x = next_x;
		}
	}
	else if(velocity_.x < 0) // 左に移動中
	{
		// collision_offset_を加えて早めに検知
		const double sensor_x = next_x - half_width - collision_offset_;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + half_width + collision_offset_;
			velocity_.x *= -1.0;
			is_facing_right_ = true;
		}
		else
		{
			pos_.x = next_x;
		}
	}
}

// 前後往復ロジック
void Enemy::UpdateBackAndForth(const Stage& stage)
{
	// 現在の移動方向に従って位置を更新
	pos_.x += velocity_.x;

	// 開始位置からの移動距離を計算
	const double distance_from_start = std::abs(pos_.x - start_pos_.x);

	// 最大移動距離に達したら方向を反転
	if(distance_from_start >= max_travel_distance_)
	{
		// 方向反転（スプライトの向きは変えない）
		velocity_.x *= -1.0;

		// 距離をリセット
		travel_distance_ = 0.0;

		// 正確な位置に補正（行き過ぎを防ぐ）
		if(velocity_.x > 0)
		{
			pos_.x = start_pos_.x - max_travel_distance_;
		}
		else
		{
			pos_.x = start_pos_.x + max_travel_distance_;
		}
	}
}

void Enemy::Draw(const Vec2& camera_offset) const
{
	if(not is_alive_) return;

	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		const Vec2 draw_pos = pos_ - camera_offset;
		const Vec2 final_draw_pos = s3d::Floor(draw_pos);

		if(is_facing_right_)
		{
			texture_asset->mirrored().drawAt(final_draw_pos);
		}
		else
		{
			texture_asset->drawAt(final_draw_pos);
		}
	}
}

#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Enemy.h"
#include "Player.h"

#include <Siv3D.hpp>
#include <variant>

Enemy::Enemy(const String& type, const Vec2& center_pos)
	: pos_(center_pos)
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
}

void Enemy::SetupProperties(const String& type)
{
	if(type == U"Fish")
	{
		behavior_ = EnemyBehavior::Patrol;
		physics_size_ = kFishPhysicsSize;
		velocity_.x = -kPatrolSpeed;
		is_facing_right_ = false;
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
		anim.texture_asset_names = { U"fish1" };
		anim.frame_duration_sec = 0.25;
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
			anim.texture_asset_names = { U"clione1" };
			anim.frame_duration_sec = 0.5;
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
		const double sensor_x = next_x + half_width;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - half_width;
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
		const double sensor_x = next_x - half_width;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + half_width;
			velocity_.x *= -1.0;
			is_facing_right_ = true;
		}
		else
		{
			pos_.x = next_x;
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

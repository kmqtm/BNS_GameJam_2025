#include "../Manager/EnemyDataManager.h"
#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Enemy.h"
#include "Player.h"

#include <Siv3D.hpp>
#include <variant>

using namespace s3d;

namespace
{
	Animation ToAnimation(const EnemyDataManager::AnimationSpec& spec)
	{
		Animation a;
		a.texture_asset_names = spec.textures;
		a.frame_duration_sec = spec.frame_duration_sec;
		a.is_looping = spec.is_looping;
		return a;
	}
}

Enemy::Enemy(const String& type, const Vec2& center_pos)
	: pos_(center_pos)
	, start_pos_(center_pos)
{
	// 仕様の取得（安全な集中管理）
	if(auto specOpt = EnemyDataManager::GetInstance().TryGetSpec(type))
	{
		const auto& spec = *specOpt;

		// behavior
		switch(spec.behavior)
		{
		case EnemyDataManager::BehaviorKind::Patrol:       behavior_ = EnemyBehavior::Patrol; break;
		case EnemyDataManager::BehaviorKind::BackAndForth: behavior_ = EnemyBehavior::BackAndForth; break;
		default:                                            behavior_ = EnemyBehavior::Stationary; break;
		}

		// physics
		physics_size_ = Vec2{ static_cast<double>(spec.physics_size.x), static_cast<double>(spec.physics_size.y) };

		// movement
		collision_offset_ = spec.collision_offset;
		if(behavior_ == EnemyBehavior::Patrol)
		{
			velocity_.x = (spec.initial_facing_right ? +spec.speed : -spec.speed);
			is_facing_right_ = spec.initial_facing_right;
		}
		else if(behavior_ == EnemyBehavior::BackAndForth)
		{
			velocity_.x = spec.initial_velocity_x;
			max_travel_distance_ = spec.max_travel_distance;
		}

		// collider
		if(spec.collider_shape == EnemyDataManager::ColliderKind::Circle)
		{
			collider_ = Collider{ Circle{ pos_, spec.collider_radius }, ColliderTag::kEnemy };
		}
		else
		{
			collider_ = Collider{ RectF{ Arg::center(pos_), spec.collider_width, spec.collider_height }, ColliderTag::kEnemy };
		}

		// animations
		for(const auto& [name, animSpec] : spec.animations)
		{
			anim_controller_.AddAnimation(name, ToAnimation(animSpec));
		}
		// 初期再生
		if(behavior_ == EnemyBehavior::Stationary)
		{
			if(spec.animations.contains(U"idle")) anim_controller_.Play(U"idle");
		}
		else
		{
			if(spec.animations.contains(U"move")) anim_controller_.Play(U"move");
		}
		return;
	}

	// JSONにない場合のフォールバック（従来の静的テーブル）
	Print << U"警告: '{}' のデータが見つからないため、デフォルト設定を使用します"_fmt(type);
	//SetupProperties(type); 削除で良いかも
	//SetupAnimations(type);

	// フォールバックのコライダー
	if(type == U"Coral_L" || type == U"Coral_R")
	{
		collider_ = Collider{ Circle{ pos_, kCoralColliderRadius }, ColliderTag::kEnemy };
	}
	else if(type == U"Fish")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kFishColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"Clione")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kClioneColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"Shark")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kSharkColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"DeepseaFish")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kDeepseaFishColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"Swimmie")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kSwimmieColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"MorayEel_L" || type == U"MorayEel_R")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kMorayEelColliderSize }, ColliderTag::kEnemy };
	}
	else if(type == U"Octoleg_L" || type == U"Octoleg_R")
	{
		collider_ = Collider{ RectF{ Arg::center(pos_), kOctolegColliderSize }, ColliderTag::kEnemy };
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

#include "../Manager/EnemyDataManager.h"
#include "../World/Stage.h"
#include "BehaviorStrategies.h"
#include "Component/Animation.h"
#include "Enemy.h"
#include "Player.h"
#include <cmath>
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
	// 仕様の取得
	auto specOpt = EnemyDataManager::GetInstance().TryGetSpec(type);
	if(not specOpt)
	{
		Print << U"エラー: '{}' のデータが見つかりません"_fmt(type);
		return;
	}

	const auto& spec = *specOpt;

	// Strategy の生成
	switch(spec.behavior)
	{
	case EnemyDataManager::BehaviorKind::Patrol:
		behavior_strategy_ = std::make_unique<PatrolBehavior>();
		break;
	case EnemyDataManager::BehaviorKind::BackAndForth:
		behavior_strategy_ = std::make_unique<BackAndForthBehavior>();
		break;
	default:
		behavior_strategy_ = std::make_unique<StationaryBehavior>();
		break;
	}

	// physics
	physics_size_ = Vec2{ static_cast<double>(spec.physics_size.x), static_cast<double>(spec.physics_size.y) };

	// movement 初期化（以前は behavior_ 判定、今は spec.behavior を直接使用）
	collision_offset_ = spec.collision_offset;
	if(spec.behavior == EnemyDataManager::BehaviorKind::Patrol)
	{
		velocity_.x = (spec.initial_facing_right ? +spec.speed : -spec.speed);
		is_facing_right_ = spec.initial_facing_right;
	}
	else if(spec.behavior == EnemyDataManager::BehaviorKind::BackAndForth)
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
	if(spec.behavior == EnemyDataManager::BehaviorKind::Stationary)
	{
		if(spec.animations.contains(U"idle"))
		{
			anim_controller_.Play(U"idle");
		}
	}
	else
	{
		if(spec.animations.contains(U"move"))
		{
			anim_controller_.Play(U"move");
		}
	}
}

void Enemy::Update(const Stage& stage, const Player& player)
{
	(void)player;

	if(not is_alive_) return;

	UpdateAI(stage);
	anim_controller_.Update();
	UpdateColliderPosition();
	HandleCollision();
}

void Enemy::UpdateAI(const Stage& stage)
{
	if(behavior_strategy_)
	{
		behavior_strategy_->Update(*this, stage);
	}
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

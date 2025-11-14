#pragma once

#include "../AI/IBehaviorStrategy.h"
#include "../Component/AnimationController.h"
#include "../Component/Collider.h"
#include "../World/Stage.h"

#include <memory>
#include <Siv3D.hpp>

// 前方宣言
class Player;

// （friend 用）派生 Strategy 前方宣言
class StationaryBehavior;
class PatrolBehavior;
class BackAndForthBehavior;

class Enemy
{
public:
	Enemy(const String& type, const Vec2& center_pos);

	void Update(const Stage& stage, const Player& player);
	void Draw(const Vec2& camera_offset) const;

	Collider& GetCollider() { return collider_; }
	const Collider& GetCollider() const { return collider_; }

	bool IsAlive() const { return is_alive_; }

	// Strategy から内部状態へアクセス
	friend class IBehaviorStrategy;
	friend class StationaryBehavior;
	friend class PatrolBehavior;
	friend class BackAndForthBehavior;

private:
	void UpdateAI(const Stage& stage);
	void UpdateColliderPosition();
	void HandleCollision();

	// Strategy ポインタ
	std::unique_ptr<IBehaviorStrategy> behavior_strategy_;

	Vec2 pos_;
	Vec2 velocity_ = Vec2::Zero();
	Vec2 physics_size_;
	double collision_offset_ = 0.0;
	bool is_alive_ = true;
	bool is_facing_right_ = false;

	// BackAndForth 用
	Vec2 start_pos_ = Vec2::Zero();
	double travel_distance_ = 0.0;
	double max_travel_distance_ = 0.0;

	AnimationController anim_controller_;
	Collider collider_{ Circle{0,0,1}, ColliderTag::kEnemy };
};

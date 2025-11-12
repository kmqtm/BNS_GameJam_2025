#pragma once

#include "../World/Stage.h"
#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

// 前方宣言
class Player;

// Strategy 前方宣言（friend指定に必要）
class IBehaviorStrategy;
class StationaryBehavior;
class PatrolBehavior;
class BackAndForthBehavior;

// 敵の振る舞いの種類
enum class EnemyBehavior
{
	Stationary,   // その場から動かない
	Patrol,       // 左右に巡回する
	BackAndForth  // 一定距離前後に往復する
};

class Enemy
{
public:
	Enemy(const String& type, const Vec2& center_pos);

	void Update(const Stage& stage, const Player& player);
	void Draw(const Vec2& camera_offset) const;

	Collider& GetCollider() { return collider_; }
	const Collider& GetCollider() const { return collider_; }

	bool IsAlive() const { return is_alive_; }

	// Strategy から内部状態へアクセスできるようにする
	friend class IBehaviorStrategy;
	friend class StationaryBehavior;
	friend class PatrolBehavior;
	friend class BackAndForthBehavior;

private:
	void UpdatePatrol(const Stage& stage);
	void UpdateBackAndForth(const Stage& stage);
	void UpdateAI(const Stage& stage);
	void UpdateColliderPosition();
	void HandleCollision();

	EnemyBehavior behavior_;

	Vec2 pos_;
	Vec2 velocity_ = Vec2::Zero();

	// 物理演算(壁との当たり判定)用のサイズ
	Vec2 physics_size_;

	// 壁との衝突検知のオフセット（大きいほど早く反転）
	double collision_offset_ = 0.0;

	bool is_alive_ = true;
	bool is_facing_right_ = false;

	// BackAndForth用の変数
	Vec2 start_pos_ = Vec2::Zero();
	double travel_distance_ = 0.0;
	double max_travel_distance_ = 0.0;

	AnimationController anim_controller_;
	Collider collider_{ Circle{0,0,1}, ColliderTag::kEnemy };
};

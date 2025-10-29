#pragma once

#include "../World/Stage.h"
#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

// 前方宣言
class Player;

// 敵の振る舞いの種類
enum class EnemyBehavior
{
	Stationary, // その場から動かない
	Patrol      // 左右に巡回する
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

private:
	void UpdatePatrol(const Stage& stage);

	EnemyBehavior behavior_;

	Vec2 pos_;
	Vec2 velocity_ = Vec2::Zero();
	Vec2 size_; // 当たり判定サイズ

	bool is_alive_ = true;
	bool is_facing_right_ = false;

	AnimationController anim_controller_;
	Collider collider_{ Circle{0,0,1}, ColliderTag::kEnemy };

	static constexpr double kPatrolSpeed = 0.4; // 巡回型敵の移動速度
};

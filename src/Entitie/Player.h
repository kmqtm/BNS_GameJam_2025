#pragma once

#include "../World/Stage.h"
#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

class Player
{
public:
	Player();

	void Update(const Stage& stage);

	void Draw(const Vec2& camera_offset) const;

	Vec2 GetPos() const;

	void SetPos(const Vec2& new_pos);

	// 地形衝突には使わないが，敵やアイテムとの当たり判定に使う
	Collider collider{ Circle{0, 0, 1.0}, ColliderTag::kPlayer };

private:
	void HandleInput();
	void UpdatePhysics(const Stage& stage);
	void UpdateAnimation();

	Vec2 pos_{ 300, 100.0 };
	Vec2 velocity_{ 0.0, 0.0 };

	bool is_moving_x_ = false;
	bool is_grounded_ = false;
	bool is_facing_right_ = false;

	// 物理パラメータ
	double horizontal_accel_{ 0.2 };
	double horizontal_speed_max_{ 0.6 };
	double friction_{ 0.70 };					// 水平方向の抵抗係数(1.0が無抵抗)
	double gravity_ = 0.05;
	double swim_power_ = -1.8;					// 水中での上昇力
	double terminal_velocity_y_ = 0.45;
	double rising_gravity_multiplier_ = 0.4;	// 上昇時の重力軽減倍率(0.0で無重力)

	AnimationController anim_controller_;
};

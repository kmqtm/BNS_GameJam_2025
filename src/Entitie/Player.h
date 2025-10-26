#pragma once

#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

class Player
{
public:
	Player();

	void Update();

	void Draw(const Vec2& camera_offset) const;

	Vec2 GetPos() const;

	void SetPos(const Vec2& new_pos);

	Collider collider{ Circle{0, 0, 1.0}, ColliderTag::Player };

private:
	void HandleInput();
	void UpdatePhysics();
	void UpdateAnimation();

	Vec2 pos_{ 0, 1.0 };
	Vec2 velocity_{ 0.0, 0.0 };

	bool is_moving_x_ = false;

	// 物理パラメータ
	double horizontal_accel_{ 0.2 };
	double horizontal_speed_max_{ 0.4 };
	double friction_{ 0.70 };					// 水平方向の抵抗係数(1.0が無抵抗)
	double gravity_ = 0.02;
	double swim_power_ = -0.7;					// 水中での上昇力
	double terminal_velocity_y_ = 0.35;
	double rising_gravity_multiplier_ = 0.8;	// 上昇時の重力軽減倍率

	AnimationController anim_controller_;
};
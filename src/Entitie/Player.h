#pragma once

#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

class Player
{
public:
	Player();

	void Update();

	void Draw() const;

	Vec2 GetPos() const;

	void SetPos(const Vec2& new_pos);

	Collider collider{ Circle{0, 0, 1.0}, ColliderTag::Player };

private:
	Vec2 pos_{ 0, 1.0 };        // 現在位置
	Vec2 velocity_{ 0.0, 0.0 }; // 現在の速度 (X, Y)

	// 物理パラメータ (水中挙動)
	// Scene::DeltaTime() を使わないフレーム単位の値

	// 水平移動
	double horizontal_accel_{ 0.4 };		// 左右への加速度
	double horizontal_speed_max_{ 1.0 };	// 左右の最大速度
	double friction_{ 0.70 };				// 水平方向の水の抵抗 (0.0で即停止, 1.0で抵抗なし)

	// 垂直移動
	double gravity_ = 0.02;				// 毎フレーム加算される重力 (沈降速度)
	double swim_power_ = -0.9;			// スペースで得る上向きの初速 (負の値)
	double terminal_velocity_y_ = 0.35;	// 最大落下速度

	// 上昇中 (velocity_.y < 0) に適用される重力係数
	// (1.0 で通常, 0.0 で無重力, 0.4 で 40% の重力)
	double rising_gravity_multiplier_ = 0.8;

	AnimationController anim_controller_;
};

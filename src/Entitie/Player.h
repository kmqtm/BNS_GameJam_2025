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
	Collider collider{ RectF{0, 0, 1.0, 1.0}, ColliderTag::kPlayer };

private:
	void HandleInput();
	void UpdatePhysics(const Stage& stage);

	void ApplyGravity();
	void MoveX(const Stage& stage);
	void MoveY(const Stage& stage);
	void UpdateColliderPosition();

	void UpdateAnimation();

	void TakeDamage();

	Vec2 pos_{ 300, 100.0 };
	Vec2 velocity_{ 0.0, 0.0 };

	bool is_moving_x_ = false;
	bool is_grounded_ = false;
	bool is_facing_right_ = false;

	bool is_invincible_ = false;
	s3d::Stopwatch invincible_timer_;
	static constexpr double kInvincibleDurationSec = 2.7;	// 無敵時間
	static constexpr int32 kBlinkIntervalMs = 300;			// 点滅の間隔
	static constexpr int32 kBlinkOnDurationMs = 150;		// 点滅中の表示時間

	bool just_took_damage_ = false;

	// 地形衝突(Physics)用のサイズ(ハーフ)
	static constexpr double kPhysicsHalfWidth = 25.0;
	static constexpr double kPhysicsHalfHeight = 62.0;

	// 敵との当たり判定(Collider)用のサイズ
	static constexpr double kColliderWidth = 60.0;
	static constexpr double kColliderHeight = 80.0;

	// 描画オフセット(スプライトの中心から左上までの距離)
	static constexpr Vec2 kDrawOffset = { 64.0, 64.0 };

	// 物理パラメータ
	double horizontal_accel_{ 0.2 };
	double horizontal_speed_max_{ 0.6 };
	double friction_{ 0.90 };					// 水平方向の抵抗係数(1.0が無抵抗)
	double gravity_ = 0.05;
	double swim_power_ = -1.8;					// 水中での上昇力
	double terminal_velocity_y_ = 0.45;			// Y軸の終端速度
	double rising_gravity_multiplier_ = 0.4;	// 上昇時の重力軽減倍率(0.0で無重力)

	// 酸素量(体力と同義)
	int32 oxygen_{ 100 };

	AnimationController anim_controller_;
};

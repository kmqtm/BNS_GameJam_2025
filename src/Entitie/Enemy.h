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

private:
	void UpdatePatrol(const Stage& stage);
	void UpdateBackAndForth(const Stage& stage);

	void SetupProperties(const String& type);
	void SetupAnimations(const String& type);

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

	static constexpr Size kFishPhysicsSize = { 32, 24 };
	static constexpr Size kCoralPhysicsSize = { 64, 64 };
	static constexpr Size kClionePhysicsSize = { 18, 24 };
	static constexpr Size kSharkPhysicsSize = { 64, 48 };
	static constexpr Size kDeepseaFishPhysicsSize = { 40, 32 };
	static constexpr Size kSwimmiePhysicsSize = { 32, 32 };
	static constexpr Size kMorayEelPhysicsSize = { 48, 32 };
	static constexpr Size kOctolegPhysicsSize = { 40, 40 };

	static constexpr Size kFishColliderSize = { 32, 24 };
	static constexpr double kCoralColliderRadius = 28.0;
	static constexpr Size kClioneColliderSize = { 18, 24 };
	static constexpr Size kSharkColliderSize = { 320, 48 };
	static constexpr Size kDeepseaFishColliderSize = { 40, 32 };
	static constexpr Size kSwimmieColliderSize = { 64, 32 };
	static constexpr Size kMorayEelColliderSize = { 192, 32 };
	static constexpr Size kOctolegColliderSize = { 288, 40 };

	// 各敵タイプの移動速度
	static constexpr double kFishSpeed = 0.67;
	static constexpr double kSharkSpeed = 0.40;
	static constexpr double kDeepseaFishSpeed = 0.30;
	static constexpr double kSwimmieSpeed = 0.75;
	static constexpr double kMorayEelSpeed = 0.60;
	static constexpr double kOctolegSpeed = 0.55;

	// 前後往復の距離（1マス = 64ピクセル）
	static constexpr double kBackAndForthDistance = 64.0;
};

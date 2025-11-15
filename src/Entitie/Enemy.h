#pragma once

/// @file Enemy.h
/// @brief 敵クラスの定義

#include "../AI/IBehaviorStrategy.h"
#include "../Component/AnimationController.h"
#include "../Component/Collider.h"
#include "../World/Stage.h"

#include <memory>
#include <Siv3D.hpp>

// 前方宣言
class Player;

// (friend 用)派生 Strategy 前方宣言
class StationaryBehavior;
class PatrolBehavior;
class BackAndForthBehavior;

/// @brief 敵を管理する
class Enemy
{
public:
	/// @brief コンストラクタ
	/// @param type 敵のタイプ
	/// @param center_pos 初期位置
	Enemy(const String& type, const Vec2& center_pos);

	/// @brief 敵を更新する
	/// @param stage ステージ情報
	/// @param player プレイヤー情報
	void Update(const Stage& stage, const Player& player);

	/// @brief 敵を描画する
	/// @param camera_offset カメラオフセット
	void Draw(const Vec2& camera_offset) const;

	/// @brief コライダーを取得する
	Collider& GetCollider() { return collider_; }

	/// @brief コライダーを取得する(const版)
	const Collider& GetCollider() const { return collider_; }

	/// @brief 敵が生存しているかを判定する
	/// @return 生存している場合はtrue
	bool IsAlive() const { return is_alive_; }

	// Strategy から内部状態へアクセス
	friend class IBehaviorStrategy;
	friend class StationaryBehavior;
	friend class PatrolBehavior;
	friend class BackAndForthBehavior;

private:
	/// @brief AIの更新処理
	/// @param stage ステージ情報
	void UpdateAI(const Stage& stage);

	/// @brief コライダー位置を更新する
	void UpdateColliderPosition();

	/// @brief 衝突処理を実行する
	void HandleCollision();

	/// @brief 行動戦略ポインタ
	std::unique_ptr<IBehaviorStrategy> behavior_strategy_;

	/// @brief 敵の位置
	Vec2 pos_;

	/// @brief 敵の速度
	Vec2 velocity_ = Vec2::Zero();

	/// @brief 敵の物理サイズ
	Vec2 physics_size_;

	/// @brief 衝突判定のオフセット
	double collision_offset_ = 0.0;

	/// @brief 敵が生存しているか
	bool is_alive_ = true;

	/// @brief 敵が右向きか
	bool is_facing_right_ = false;

	/// @brief 往復移動の開始位置
	Vec2 start_pos_ = Vec2::Zero();

	/// @brief 往復移動の現在の移動距離
	double travel_distance_ = 0.0;

	/// @brief 往復移動の最大移動距離
	double max_travel_distance_ = 0.0;

	/// @brief アニメーションコントローラー
	AnimationController anim_controller_;

	/// @brief コライダー
	Collider collider_{ Circle{0,0,1}, ColliderTag::kEnemy };
};

#pragma once

/// @file OxygenSpot.h
/// @brief 酸素スポットクラスの定義

#include "../Component/AnimationController.h"
#include "../Component/Collider.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

/// @brief 酸素スポットを管理する
class OxygenSpot
{
public:
	/// @brief コンストラクタ
	/// @param center_pos 初期位置
	/// @param size サイズ
	OxygenSpot(const Vec2& center_pos, const Vec2& size);

	/// @brief 酸素スポットを更新する
	void Update();

	/// @brief 酸素スポットを描画する
	/// @param camera_offset カメラオフセット
	void Draw(const Vec2& camera_offset) const;

	/// @brief 位置を取得する
	/// @return 酸素スポットの位置
	Vec2 GetPos() const;

	/// @brief コライダーを取得する
	Collider& GetCollider() { return collider_; }

	/// @brief コライダーを取得する(const版)
	const Collider& GetCollider() const { return collider_; }

private:
	/// @brief 位置
	Vec2 pos_;

	/// @brief 当たり判定サイズ
	Vec2 size_;

	/// @brief アニメーションコントローラー
	AnimationController anim_controller_;

	/// @brief 当たり判定
	Collider collider_;

	/// @brief アニメーション初期化を実行する
	void SetupAnimation();

	/// @brief コライダーの中心位置を更新する
	void UpdateColliderCenter();
};

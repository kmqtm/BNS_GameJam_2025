#pragma once

/// @file Collider.h
/// @brief 衝突判定用のコンポーネント定義

#include <Siv3D.hpp>
#include <variant>

/// @brief 衝突判定のタグ
///
/// ゲーム内のオブジェクトの種類を識別する
enum class ColliderTag
{
	kPlayer,   ///< プレイヤー
	kEnemy,    ///< 敵
	kOxygen,   ///< 酸素
	kWall,     ///< 壁
};

/// @brief 衝突判定で使用可能な図形
using ShapeVariant = std::variant<s3d::Circle, s3d::RectF, s3d::Line>;

/// @brief 衝突判定を管理する構造体
struct Collider
{
public:
	/// @brief 衝突判定用の図形
	ShapeVariant shape;

	/// @brief ワールド座標からのオフセット位置
	s3d::Vec2 offset;

	/// @brief このコライダーのタグ
	ColliderTag tag;

	/// @brief 衝突した相手のタグのリスト
	s3d::Array<ColliderTag> collided_tags;

	/// @brief 現在衝突状態にあるか
	bool is_colliding = false;

	/// @brief コンストラクタ
	/// @param initial_shape 初期図形
	/// @param initial_tag コライダーのタグ
	Collider(const ShapeVariant& initial_shape, ColliderTag initial_tag)
		: shape{ initial_shape }, offset{ 0, 0 }, tag{ initial_tag }
	{
	}

	/// @brief 衝突判定結果をクリアする
	void ClearCollisionResult()
	{
		collided_tags.clear();
		is_colliding = false;
	}
};

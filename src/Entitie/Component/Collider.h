#pragma once

#include <Siv3D.hpp>
#include <variant>

// 当たり判定の種類を識別するためのタグ
enum class ColliderTag
{
	Player,
	Enemy,
	Oxigen,
	Wall,
};

// Circle, RectF, Line のいずれかの形状を代入できる
using ShapeVariant = std::variant<s3d::Circle, s3d::RectF, s3d::Line>;

class Collider
{
public:
	// このコライダーの2D形状
	ShapeVariant shape;

	// Colliderを持つオブジェクトの2D座標オフセット(通常は {0, 0} で良い)
	s3d::Vec2 offset;

	// このコライダーの種類を識別するタグ
	ColliderTag tag;

	// 衝突結果を格納する変数

	// このフレームで衝突した相手のタグリスト
	s3d::Array<ColliderTag> collided_tags;

	// 衝突中かどうかを示すフラグ
	bool is_colliding = false;

	Collider(const ShapeVariant& initial_shape, ColliderTag initial_tag)
		: shape{ initial_shape }, offset{ 0, 0 }, tag{ initial_tag }
	{
	}

	// 毎フレームの当たり判定チェック前に結果をリセット
	void ClearCollisionResult()
	{
		collided_tags.clear();
		is_colliding = false;
	}
};

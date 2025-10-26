#pragma once

#include <Siv3D.hpp>
#include <variant>

enum class ColliderTag
{
	kPlayer,
	kEnemy,
	kOxygen,
	kWall,
};

using ShapeVariant = std::variant<s3d::Circle, s3d::RectF, s3d::Line>;

struct Collider
{
public:
	ShapeVariant shape;
	s3d::Vec2 offset;
	ColliderTag tag;
	s3d::Array<ColliderTag> collided_tags;
	bool is_colliding = false;

	Collider(const ShapeVariant& initial_shape, ColliderTag initial_tag)
		: shape{ initial_shape }, offset{ 0, 0 }, tag{ initial_tag }
	{
	}

	void ClearCollisionResult()
	{
		collided_tags.clear();
		is_colliding = false;
	}
};
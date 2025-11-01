#pragma once

#include "../World/Stage.h"
#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

class OxygenSpot
{
public:
	OxygenSpot(const Vec2& center_pos, const Vec2& size);

	void Update();

	void Draw(const Vec2& camera_offset) const;

	Vec2 GetPos() const;

	// GameSceneが当たり判定に使う
	Collider& GetCollider() { return collider_; }
	const Collider& GetCollider() const { return collider_; }

private:
	Vec2 pos_;
	Vec2 size_; // 当たり判定サイズ

	AnimationController anim_controller_;
	Collider collider_; // 当たり判定
};

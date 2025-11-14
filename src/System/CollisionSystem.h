#pragma once

#include "../Component/Collider.h"

#include <cstdint>
#include <Siv3D.hpp>
#include <variant>
#include <vector>

class CollisionSystem
{
public:
	// Playerのコライダーを登録（1つのみ）
	void RegisterPlayer(Collider* player_collider);

	// Player以外のコライダーを登録（Enemy、OxygenSpotなど）
	void RegisterOther(Collider* other_collider, uint32_t entity_id);

	/// 登録された全ての Collider の衝突結果をクリア
	void ClearResults();

	// Player vs Other の衝突判定のみを行う（O(N)）
	void ResolveCollisions();

	// 登録されているColliderをすべてクリア
	void Clear();

private:
	struct ColliderEntry
	{
		Collider* collider;
		uint32_t entity_id;
	};

	Collider* player_collider_ = nullptr;
	std::vector<ColliderEntry> other_colliders_;

	// 2つのShapeVariant同士の衝突判定を行う
	bool CheckIntersection(const ShapeVariant& shape1, const ShapeVariant& shape2) const;
};

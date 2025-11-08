#pragma once

#include "../Entitie/Component/Collider.h"

#include <cstdint>
#include <Siv3D.hpp>
#include <variant>
#include <vector>

class CollisionManager
{
public:
	// Colliderとそれが属するエンティティIDを登録
	void RegisterCollider(Collider* collider, uint32_t entity_id);

	// 登録された全ての Collider の衝突結果をクリア
	void ClearResults();

	// 登録された全てのCollider同士の衝突判定を行い，結果を各 Collider に記録
	void ResolveCollisions();

	/// 登録されているColliderをすべてクリア
	void Clear();

private:
	struct ColliderEntry
	{
		Collider* collider;
		uint32_t entity_id;
	};

	std::vector<ColliderEntry> colliders_;

	// 2つのShapeVariant同士の衝突判定を行う
	bool CheckIntersection(const ShapeVariant& shape1, const ShapeVariant& shape2) const;
};

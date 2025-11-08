#include "CollisionManager.h"

void CollisionManager::RegisterCollider(Collider* collider, uint32_t entity_id)
{
	if(collider)
	{
		colliders_.push_back({ collider, entity_id });
	}
}

void CollisionManager::ClearResults()
{
	for(auto& entry : colliders_)
	{
		if(entry.collider)
		{
			entry.collider->ClearCollisionResult();
		}
	}
}

void CollisionManager::ResolveCollisions()
{
	const size_t count = colliders_.size();

	for(size_t i = 0; i < count; ++i)
	{
		for(size_t j = i + 1; j < count; ++j)
		{
			auto& entry_a = colliders_[i];
			auto& entry_b = colliders_[j];

			if(!entry_a.collider || !entry_b.collider)
			{
				continue;
			}

			// 2つの Collider の shape 同士で衝突判定
			const bool is_colliding = CheckIntersection(entry_a.collider->shape, entry_b.collider->shape);

			if(is_colliding)
			{
				// A に B の情報を追加
				entry_a.collider->is_colliding = true;
				entry_a.collider->collided_tags.push_back(entry_b.collider->tag);

				// B に A の情報を追加
				entry_b.collider->is_colliding = true;
				entry_b.collider->collided_tags.push_back(entry_a.collider->tag);
			}
		}
	}
}

void CollisionManager::Clear()
{
	colliders_.clear();
}

bool CollisionManager::CheckIntersection(const ShapeVariant& shape1, const ShapeVariant& shape2) const
{
	return std::visit([&](const auto& s1)
					  {
						  return std::visit([&](const auto& s2)
											{
												return s1.intersects(s2);
											}, shape2);
					  }, shape1);
}

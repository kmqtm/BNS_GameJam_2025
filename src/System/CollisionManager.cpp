#include "CollisionManager.h"

void CollisionManager::RegisterPlayer(Collider* player_collider)
{
	player_collider_ = player_collider;
}

void CollisionManager::RegisterOther(Collider* other_collider, uint32_t entity_id)
{
	if(other_collider)
	{
		other_colliders_.push_back({ other_collider, entity_id });
	}
}

void CollisionManager::ClearResults()
{
	// Playerの衝突結果をクリア
	if(player_collider_)
	{
		player_collider_->ClearCollisionResult();
	}

	// その他のコライダーの衝突結果をクリア
	for(auto& entry : other_colliders_)
	{
		if(entry.collider)
		{
			entry.collider->ClearCollisionResult();
		}
	}
}

void CollisionManager::ResolveCollisions()
{
	// Playerが登録されていない場合は処理しない
	if(!player_collider_)
	{
		return;
	}

	// Player vs Other の O(N) 衝突判定
	for(auto& other_entry : other_colliders_)
	{
		if(!other_entry.collider)
		{
			continue;
		}

		// PlayerとOtherの衝突判定
		const bool is_colliding = CheckIntersection(player_collider_->shape, other_entry.collider->shape);

		if(is_colliding)
		{
			// Playerに衝突情報を追加
			player_collider_->is_colliding = true;
			player_collider_->collided_tags.push_back(other_entry.collider->tag);

			// Otherに衝突情報を追加
			other_entry.collider->is_colliding = true;
			other_entry.collider->collided_tags.push_back(player_collider_->tag);
		}
	}
}

void CollisionManager::Clear()
{
	player_collider_ = nullptr;
	other_colliders_.clear();
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

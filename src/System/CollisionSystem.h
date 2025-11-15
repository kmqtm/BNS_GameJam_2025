#pragma once

/// @file CollisionSystem.h
/// @brief 衝突判定システムの定義

#include "../Component/Collider.h"

#include <cstdint>
#include <Siv3D.hpp>
#include <variant>
#include <vector>

/// @brief プレイヤーと他のオブジェクト間の衝突判定を管理する
class CollisionSystem
{
public:
	/// @brief プレイヤーのコライダーを登録する
	/// @param player_collider プレイヤーのコライダー
	void RegisterPlayer(Collider* player_collider);

	/// @brief プレイヤー以外のコライダーを登録する
	/// @param other_collider 登録するコライダー
	/// @param entity_id エンティティID
	void RegisterOther(Collider* other_collider, uint32_t entity_id);

	/// @brief 登録されたすべてのコライダーの衝突結果をクリアする
	void ClearResults();

	/// @brief プレイヤーと他のオブジェクトの衝突判定を実行する
	void ResolveCollisions();

	/// @brief 登録されているすべてのコライダーをクリアする
	void Clear();

private:
	/// @brief コライダー登録用の構造体
	struct ColliderEntry
	{
		Collider* collider;  ///< コライダーポインタ
		uint32_t entity_id;  ///< エンティティID
	};

	/// @brief プレイヤーのコライダー
	Collider* player_collider_ = nullptr;

	/// @brief その他のコライダーリスト
	std::vector<ColliderEntry> other_colliders_;

	/// @brief 2つの図形の衝突判定を実行する
	/// @param shape1 図形1
	/// @param shape2 図形2
	/// @return 衝突している場合はtrue
	bool CheckIntersection(const ShapeVariant& shape1, const ShapeVariant& shape2) const;
};

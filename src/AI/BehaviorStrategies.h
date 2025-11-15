#pragma once

/// @file BehaviorStrategies.h
/// @brief 敵の行動戦略具象クラスの定義
///
/// IBehaviorStrategyを継承した複数の行動パターンを実装している

#include "IBehaviorStrategy.h"

/// @brief その場から動かない敵の行動戦略
class StationaryBehavior final : public IBehaviorStrategy
{
public:
	/// @brief 敵の行動を更新する
	/// @param enemy 対象の敵オブジェクト
	/// @param stage ステージ情報
	void Update(Enemy& enemy, const Stage& stage) override;
};

/// @brief 左右に巡回する敵の行動戦略
class PatrolBehavior final : public IBehaviorStrategy
{
public:
	/// @brief 敵の行動を更新する
	/// @param enemy 対象の敵オブジェクト
	/// @param stage ステージ情報
	void Update(Enemy& enemy, const Stage& stage) override;
};

/// @brief 一定距離前後に往復する敵の行動戦略
class BackAndForthBehavior final : public IBehaviorStrategy
{
public:
	/// @brief 敵の行動を更新する
	/// @param enemy 対象の敵オブジェクト
	/// @param stage ステージ情報
	void Update(Enemy& enemy, const Stage& stage) override;
};

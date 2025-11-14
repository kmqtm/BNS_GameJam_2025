#pragma once
#include "IBehaviorStrategy.h"

// その場から動かない
class StationaryBehavior final : public IBehaviorStrategy
{
public:
	void Update(Enemy& enemy, const Stage& stage) override;
};

// 左右に巡回する
class PatrolBehavior final : public IBehaviorStrategy
{
public:
	void Update(Enemy& enemy, const Stage& stage) override;
};

// 一定距離前後に往復する
class BackAndForthBehavior final : public IBehaviorStrategy
{
public:
	void Update(Enemy& enemy, const Stage& stage) override;
};

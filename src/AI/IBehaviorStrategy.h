#pragma once

// 前方宣言(依存最小化)
class Enemy;
class Stage;

// AIのStrategyパターン用インターフェース
class IBehaviorStrategy
{
public:
	virtual ~IBehaviorStrategy() = default;

	// 各戦略ごとの更新処理
	virtual void Update(Enemy& enemy, const Stage& stage) = 0;
};

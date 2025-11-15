#pragma once

/// @file IBehaviorStrategy.h
/// @brief 敵の行動戦略インターフェースの定義
///
/// Strategyパターンを用いた敵の行動制御のための基底クラスを定義している

// 前方宣言(依存最小化)
class Enemy;
class Stage;

/// @brief 敵の行動戦略を定義するインターフェース
///
/// Strategyパターンを使用し，敵の行動ロジックを戦略として実装可能にしている
/// 各具象戦略クラスはこのインターフェースを継承して，特定の行動パターンを定義している
class IBehaviorStrategy
{
public:
	/// @brief デストラクタ
	virtual ~IBehaviorStrategy() = default;

	/// @brief 各戦略ごとの更新処理
	/// @param enemy 対象の敵オブジェクト
	/// @param stage ステージ情報
	virtual void Update(Enemy& enemy, const Stage& stage) = 0;
};

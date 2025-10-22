#pragma once

#include "Component/AnimationController.h"
#include "Component/Collider.h"

#include <Siv3D.hpp>

// プレイヤークラス
// 自身の状態更新や描画処理を担当する
class Player {
public:
	Player();

	// 入力処理に基づき，プレイヤーの状態を更新
	void Update();

	// プレイヤーを描画
	void Draw() const;

	// プレイヤーの現在位置を取得
	Vec2 GetPos() const;

	// プレイヤーの位置を設定
	void SetPos(const Vec2 &new_pos);

	Collider collider{Circle{0, 0, 1.0}, ColliderTag::Player};

private:
	Vec2 pos_{0, 1.0};  // 初期位置
	double speed_{0.1};     // 移動速度

	AnimationController anim_controller_;
};

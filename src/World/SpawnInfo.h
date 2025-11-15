#pragma once

/// @file SpawnInfo.h
/// @brief スポーン情報の定義

#include <Siv3D.hpp>

/// @brief Tiledエディタから読み込まれたスポーン情報
///
/// オブジェクトレイヤーで定義された敵や酸素スポットなどの
/// 配置位置と属性情報を保持する
struct SpawnInfo
{
	/// @brief オブジェクトの種類(Tiledで設定されたクラス)
	String type;

	/// @brief オブジェクトの位置(ワールド座標,左上基準)
	Vec2 pos;

	/// @brief オブジェクトのサイズ(幅と高さ)
	Vec2 size;
};

#pragma once

/// @file Utility.h
/// @brief ユーティリティ関数の定義

#include <cmath>
#include <Siv3D.hpp>

/// @brief ユーティリティ関数の名前空間
namespace Utility
{
	/// @brief Vec2を最も近い整数座標に丸める
	/// @param v 丸める対象のベクトル
	/// @return 丸められたベクトル
	inline Vec2 RoundVec2(const Vec2& v)
	{
		return Vec2(std::round(v.x), std::round(v.y));
	}
}

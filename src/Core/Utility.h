#pragma once
#include <Siv3D.hpp>
#include <cmath>

namespace Utility {

	// Vec2 を最も近い整数座標に丸める (ピクセルスナップ用)
	inline Vec2 RoundVec2(const Vec2 &v)
	{
		return Vec2(std::round(v.x), std::round(v.y));
	}
}

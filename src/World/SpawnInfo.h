#pragma once

#include <Siv3D.hpp>

struct SpawnInfo
{
	// Tiledで設定された型(Tiledエディタ上ではClassと表記されている)
	String type;

	// Tiled上の位置(デフォルトでは左上座標)
	Vec2 pos;

	// Tiled上のサイズ(幅と高さ)
	Vec2 size;
};

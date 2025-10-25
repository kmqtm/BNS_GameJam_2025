# include "Stage.h"
#include <Siv3D.hpp>
#include <cmath>

Stage::Stage(const FilePath& jsonPath, const FilePath& tilesetPath)
	: m_tileTexture(tilesetPath)
{
	loadFromJSON(jsonPath);

	const int32 tilesX = (m_tileTexture.width() / m_tileSize);
	const int32 tilesY = (m_tileTexture.height() / m_tileSize);

	for (int32 ty = 0; ty < tilesY; ++ty)
	{
		for (int32 tx = 0; tx < tilesX; ++tx)
		{
			const double x = tx * m_tileSize;
			const double y = ty * m_tileSize;
			const double size = m_tileSize;

			m_tileRegions << m_tileTexture(x, y, size, size);
		}
	}
}

void Stage::loadFromJSON(const FilePath& jsonPath)
{
	const JSON json = JSON::Load(jsonPath);
	if (not json)
	{
		throw Error{ U"Stage::loadFromJSON(): JSONファイルの読み込みに失敗しました → {}"_fmt(jsonPath) };
	}

	m_mapWidth = json[U"width"].get<int32>();
	m_mapHeight = json[U"height"].get<int32>();
	m_tileSize = json[U"tilewidth"].get<int32>();

	for (const auto& layer : json[U"layers"].arrayView())
	{
		if (layer[U"type"].getString() == U"tilelayer")
		{
			TileMapLayer newLayer;
			newLayer.name = layer[U"name"].getString();

			Grid<int32> grid(m_mapWidth, m_mapHeight);
			const auto& data = layer[U"data"].arrayView();

			for (size_t i = 0; i < layer[U"data"].size(); ++i)
			{
				const int32 x = i % m_mapWidth;
				const int32 y = i / m_mapWidth;
				grid[y][x] = data[i].get<int32>();
			}

			newLayer.tiles = std::move(grid);
			m_layers << std::move(newLayer);
		}
	}
}

static inline Vec2 roundVec2(const Vec2& v)
{
	return Vec2(std::round(v.x), std::round(v.y));
}

void Stage::draw(const Vec2& offset) const
{
	const ScopedRenderStates2D sampler{ SamplerState::ClampNearest };

	// カメラオフセットを整数ピクセルにスナップ
	const Vec2 snappedOffset = roundVec2(offset);

	for (const auto& layer : m_layers)
	{
		for (size_t y = 0; y < layer.tiles.height(); ++y)
		{
			for (size_t x = 0; x < layer.tiles.width(); ++x)
			{
				const int32 tileID = layer.tiles[y][x];
				if (tileID <= 0) continue;

				// カメラの中心位置を原点にするために引き算
				const Vec2 drawPos = Vec2{ x * m_tileSize, y * m_tileSize } - snappedOffset;
				m_tileRegions[tileID - 1].draw(drawPos);
			}
		}
	}
}


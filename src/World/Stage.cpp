#include "../Core/Utility.h"
# include "Stage.h"

#include <cmath>
#include <Siv3D.hpp>

Stage::Stage(const FilePath& jsonPath, const FilePath& tilesetPath)
	: tile_texture_(tilesetPath)
{
	LoadFromJSON(jsonPath);

	const int32 tilesX = (tile_texture_.width() / tile_size_);
	const int32 tilesY = (tile_texture_.height() / tile_size_);

	for(int32 ty = 0; ty < tilesY; ++ty)
	{
		for(int32 tx = 0; tx < tilesX; ++tx)
		{
			const double x = tx * tile_size_;
			const double y = ty * tile_size_;
			const double size = tile_size_;

			tile_regions_ << tile_texture_(x, y, size, size);
		}
	}
}

void Stage::LoadFromJSON(const FilePath& jsonPath)
{
	const JSON json = JSON::Load(jsonPath);
	if(not json)
	{
		throw Error{ U"Stage::loadFromJSON(): JSONファイルの読み込みに失敗しました → {}"_fmt(jsonPath) };
	}

	map_width_ = json[U"width"].get<int32>();
	map_height_ = json[U"height"].get<int32>();
	tile_size_ = json[U"tilewidth"].get<int32>();

	for(const auto& layer : json[U"layers"].arrayView())
	{
		if(layer[U"type"].getString() == U"tilelayer")
		{
			TileMapLayer newLayer;
			newLayer.name = layer[U"name"].getString();

			Grid<int32> grid(map_width_, map_height_);
			const auto& data = layer[U"data"].arrayView();

			for(size_t i = 0; i < layer[U"data"].size(); ++i)
			{
				const int32 x = static_cast<s3d::int32>(i) % map_width_;
				const int32 y = static_cast<s3d::int32>(i) / map_width_;
				grid[y][x] = data[i].get<int32>();
			}

			newLayer.tiles = std::move(grid);
			layers_ << std::move(newLayer);
		}
	}
}

void Stage::Draw(const Vec2& offset) const
{
	const ScopedRenderStates2D sampler{ SamplerState::ClampNearest };

	// カメラオフセットを整数ピクセルにスナップ
	const Vec2 snappedOffset = Utility::RoundVec2(offset);

	for(const auto& layer : layers_)
	{
		for(size_t y = 0; y < layer.tiles.height(); ++y)
		{
			for(size_t x = 0; x < layer.tiles.width(); ++x)
			{
				const int32 tileID = layer.tiles[y][x];
				if(tileID <= 0) continue;

				// カメラの中心位置を原点にするために引き算
				const Vec2 drawPos = Vec2{ x * tile_size_, y * tile_size_ } - snappedOffset;
				tile_regions_[tileID - 1].draw(drawPos);
			}
		}
	}
}


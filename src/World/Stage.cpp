#include "../Core/Utility.h"
# include "Stage.h"

#include <cmath>
#include <Siv3D.hpp>

Stage::Stage(const FilePath& json_path, const FilePath& tileset_path, const String& collision_layer_name)
	: tile_texture_(tileset_path)
	, collision_layer_name_(collision_layer_name)
{
	LoadFromJson(json_path);
	CreateTileRegions();

	// 当たり判定レイヤーを検索してポインタを保持
	// LoadFromJson の後で実行
	collision_layer_ = nullptr;
	for(const auto& layer : layers_)
	{
		if(layer.name == collision_layer_name_)
		{
			collision_layer_ = &layer;
			break;
		}
	}

	// 見つからなかった場合
	if(not collision_layer_)
	{
		throw Error{ U"Stage: 当たり判定レイヤー '{}' が見つかりませんでした．"_fmt(collision_layer_name_) };
	}
}

void Stage::CreateTileRegions()
{
	const int32 tiles_x = (tile_texture_.width() / tile_size_);
	const int32 tiles_y = (tile_texture_.height() / tile_size_);

	for(int32 tile_y = 0; tile_y < tiles_y; ++tile_y)
	{
		for(int32 tile_x = 0; tile_x < tiles_x; ++tile_x)
		{
			const double x = tile_x * tile_size_;
			const double y = tile_y * tile_size_;
			const double size = tile_size_;

			tile_regions_ << tile_texture_(x, y, size, size);
		}
	}
}

void Stage::LoadFromJson(const FilePath& json_path)
{
	const JSON json = JSON::Load(json_path);
	if(not json)
	{
		throw Error{ U"Stage::LoadFromJson(): JSONファイルの読み込みに失敗しました → {}"_fmt(json_path) };
	}

	map_width_ = json[U"width"].get<int32>();
	map_height_ = json[U"height"].get<int32>();
	tile_size_ = json[U"tilewidth"].get<int32>();

	for(const auto& layer : json[U"layers"].arrayView())
	{
		if(layer[U"type"].getString() == U"tilelayer")
		{
			ParseTileLayer(layer);
		}
	}
}

void Stage::ParseTileLayer(const JSON& layer_json)
{
	TileMapLayer new_layer;
	new_layer.name = layer_json[U"name"].getString();

	Grid<int32> grid(map_width_, map_height_);
	const auto& data = layer_json[U"data"].arrayView();

	for(size_t i = 0; i < layer_json[U"data"].size(); ++i)
	{
		const int32 x = static_cast<s3d::int32>(i) % map_width_;
		const int32 y = static_cast<s3d::int32>(i) / map_width_;
		grid[y][x] = data[i].get<int32>();
	}

	new_layer.tiles = std::move(grid);
	layers_ << std::move(new_layer);
}

void Stage::Draw(const Vec2& camera_offset, const RectF& view_rect) const
{
	const ScopedRenderStates2D sampler{ SamplerState::ClampNearest };

	const int32 start_x = Max(0, static_cast<int32>(view_rect.x / tile_size_));
	const int32 start_y = Max(0, static_cast<int32>(view_rect.y / tile_size_));
	const int32 end_x = Min(map_width_, static_cast<int32>(std::ceil(view_rect.tr().x / tile_size_)));
	const int32 end_y = Min(map_height_, static_cast<int32>(std::ceil(view_rect.br().y / tile_size_)));

	for(const auto& layer : layers_)
	{
		// 当たり判定レイヤーは描画しない
		if(layer.name == collision_layer_name_) continue;

		for(int32 y = start_y; y < end_y; ++y)
		{
			for(int32 x = start_x; x < end_x; ++x)
			{
				const int32 tile_id = layer.tiles[y][x];
				if(tile_id <= 0) continue;

				const Vec2 world_pos = Vec2{ x * tile_size_, y * tile_size_ };
				const Vec2 draw_pos = world_pos - camera_offset;
				tile_regions_[tile_id - 1].draw(draw_pos);
			}
		}
	}
}

// ワールド座標 (px) から当たり判定をチェックする関数
bool Stage::IsSolid(double world_x, double world_y) const
{
	// 当たり判定レイヤーがコンストラクタで正常に設定されていない場合は常に「壁なし」
	if(not collision_layer_)
	{
		return false;
	}

	// ワールド座標(px)をタイル座標(グリッドのインデックス)に変換
	// (floorを使いマイナス座標に対応)
	const int32 tile_x = static_cast<int32>(std::floor(world_x / tile_size_));
	const int32 tile_y = static_cast<int32>(std::floor(world_y / tile_size_));

	// マップの範囲外かチェック
	// (範囲外は「壁」として扱う)
	if((tile_x < 0) || (tile_x >= map_width_) || (tile_y < 0) || (tile_y >= map_height_))
	{
		return true;
	}

	// ID > 0 なら「壁あり」と判定
	return (collision_layer_->tiles[tile_y][tile_x] > 0);
}

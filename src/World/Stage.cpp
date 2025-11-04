#include "../Core/Utility.h"
#include "SpawnInfo.h"
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
	FindCollisionLayer();

	// 見つからなかった場合
	if(not collision_layer_)
	{
		throw Error{ U"Stage: 当たり判定レイヤー '{}' が見つかりませんでした．"_fmt(collision_layer_name_) };
	}
}

void Stage::FindCollisionLayer()
{
	collision_layer_ = nullptr;
	for(const auto& layer : layers_)
	{
		if(layer.name == collision_layer_name_)
		{
			collision_layer_ = &layer;
			break;
		}
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
		const String type = layer[U"type"].getString();
		if(type == U"tilelayer")
		{
			ParseTileLayer(layer);
		}
		else if(type == U"objectgroup")
		{
			ParseObjectLayer(layer);
		}
	}
}

void Stage::ParseTileLayer(const JSON& layer_json)
{
	TileMapLayer new_layer;
	new_layer.name = layer_json[U"name"].getString();

	Grid<int32> grid(map_width_, map_height_);
	const auto& data = layer_json[U"data"].arrayView();

	const size_t data_size = layer_json[U"data"].size();
	for(size_t i = 0; i < data_size; ++i)
	{
		const int32 x = static_cast<s3d::int32>(i) % map_width_;
		const int32 y = static_cast<s3d::int32>(i) / map_width_;
		grid[y][x] = data[i].get<int32>();
	}

	new_layer.tiles = std::move(grid);
	layers_ << std::move(new_layer);
}

void Stage::ParseObjectLayer(const JSON& layer_json)
{
	if(layer_json[U"name"].getString() != U"spawn_layer")
	{
		return;
	}

	for(const auto& object : layer_json[U"objects"].arrayView())
	{
		SpawnInfo info;
		info.type = object[U"type"].getString();
		info.pos = { object[U"x"].get<double>(), object[U"y"].get<double>() };
		info.size = { object[U"width"].get<double>(), object[U"height"].get<double>() };

		spawn_points_.push_back(info);
	}
}

const s3d::Array<SpawnInfo>& Stage::GetSpawnPoints() const
{
	return spawn_points_;
}

void Stage::ComputeDrawRange(const RectF& view_rect, int32& out_start_x, int32& out_start_y, int32& out_end_x, int32& out_end_y) const
{
	// 描画範囲の計算にstd::floorを使用し，負の座標でも正確に動作するようにする
	out_start_x = Max(0, static_cast<int32>(std::floor(view_rect.x / tile_size_)));
	out_start_y = Max(0, static_cast<int32>(std::floor(view_rect.y / tile_size_)));
	out_end_x = Min(map_width_, static_cast<int32>(std::ceil(view_rect.tr().x / tile_size_)));
	out_end_y = Min(map_height_, static_cast<int32>(std::ceil(view_rect.br().y / tile_size_)));
}

void Stage::DrawLayerTiles(const TileMapLayer& layer, int32 start_x, int32 start_y, int32 end_x, int32 end_y, const Vec2& camera_offset) const
{
	for(int32 y = start_y; y < end_y; ++y)
	{
		for(int32 x = start_x; x < end_x; ++x)
		{
			const int32 tile_id = layer.tiles[y][x];
			if(tile_id <= 0) continue;

			const Vec2 world_pos = Vec2{ x * tile_size_, y * tile_size_ };
			const Vec2 draw_pos = world_pos - camera_offset;

			// 整数にスナップ
			tile_regions_[tile_id - 1].draw(s3d::Floor(draw_pos));
		}
	}
}

void Stage::Draw(const Vec2& camera_offset, const RectF& view_rect) const
{
	const ScopedRenderStates2D sampler{ SamplerState::ClampNearest };

	int32 start_x, start_y, end_x, end_y;
	ComputeDrawRange(view_rect, start_x, start_y, end_x, end_y);

	for(const auto& layer : layers_)
	{
		// 当たり判定レイヤーは描画しない
		if(layer.name == collision_layer_name_) continue;

		DrawLayerTiles(layer, start_x, start_y, end_x, end_y, camera_offset);
	}
}

// ワールド座標(px)から当たり判定をチェックする関数
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
	// (範囲外は壁として扱う)
	if((tile_x < 0) || (tile_x >= map_width_) || (tile_y < 0) || (tile_y >= map_height_))
	{
		return true;
	}

	// ID > 0 なら「壁あり」と判定
	return (collision_layer_->tiles[tile_y][tile_x] > 0);
}

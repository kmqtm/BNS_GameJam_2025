#pragma once
# include <Siv3D.hpp>

struct TileMapLayer
{
	String name;
	Grid<int32> tiles;
};

// Tiledから出力したJSONを元にマップ全体を管理するクラス
class Stage
{
public:
	Stage(const FilePath& json_path, const FilePath& tileset_path);

	void Draw(const Vec2& camera_offset, const RectF& view_rect) const;

	int32 GetWidth() const { return map_width_; }
	int32 GetHeight() const { return map_height_; }
	int32 GetTileSize() const { return tile_size_; }

private:
	int32 map_width_ = 0;
	int32 map_height_ = 0;
	int32 tile_size_ = 16;

	Array<TileMapLayer> layers_;

	Texture tile_texture_;
	Array<TextureRegion> tile_regions_;

	void LoadFromJson(const FilePath& json_path);

	void ParseTileLayer(const JSON& layer_json);

	void CreateTileRegions();
};
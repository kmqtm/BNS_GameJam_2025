#pragma once
# include <Siv3D.hpp>

struct TileMapLayer
{
	String name;
	Grid<int32> tiles;
};

// Tiled で作られたマップ全体を管理するクラス
class Stage
{
public:
	// jsonPath: Tiled の JSON ファイルパス
	// tilesetPath: タイルセット画像のパス
	Stage(const FilePath& jsonPath, const FilePath& tilesetPath);

	void Draw(const Vec2& offset = Vec2{ 0, 0 }) const;

	int32 GetWidth() const { return map_width_; }

	int32 GetHeight() const { return map_height_; }

	int32 GetTileSize() const { return tile_size_; }

private:
	// 基本情報
	int32 map_width_ = 0;
	int32 map_height_ = 0;
	int32 tile_size_ = 16;

	// レイヤー情報
	Array<TileMapLayer> layers_;

	// タイルセット画像と切り出し
	Texture tile_texture_;
	Array<TextureRegion> tile_regions_;

	// 内部関数: Tiled JSON をロード
	void LoadFromJSON(const FilePath& jsonPath);
};

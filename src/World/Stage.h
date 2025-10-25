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

	void draw(const Vec2& offset = Vec2{ 0, 0 }) const;

	int32 getWidth() const { return m_mapWidth; }

	int32 getHeight() const { return m_mapHeight; }

	int32 getTileSize() const { return m_tileSize; }

private:
	// 基本情報
	int32 m_mapWidth = 0;
	int32 m_mapHeight = 0;
	int32 m_tileSize = 16;

	// レイヤー情報
	Array<TileMapLayer> m_layers;

	// タイルセット画像と切り出し
	Texture m_tileTexture;
	Array<TextureRegion> m_tileRegions;

	// 内部関数: Tiled JSON をロード
	void loadFromJSON(const FilePath& jsonPath);
};

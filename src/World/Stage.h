#pragma once
# include <Siv3D.hpp>

/// @brief 1つのタイルレイヤー
struct TileMapLayer
{
	String name;
	Grid<int32> tiles;
};

/// @brief Tiled で作られたマップ全体を管理するクラス
class Stage
{
public:
	/// @brief コンストラクタ
	/// @param jsonPath Tiled の JSON ファイルパス
	/// @param tilesetPath タイルセット画像のパス
	Stage(const FilePath& jsonPath, const FilePath& tilesetPath);

	/// @brief マップを描画
	/// @param offset 描画オフセット位置（スクロールなどに利用）
	void draw(const Vec2& offset = Vec2{ 0, 0 }) const;

	/// @brief マップの幅（タイル単位）
	int32 getWidth() const { return m_mapWidth; }

	/// @brief マップの高さ（タイル単位）
	int32 getHeight() const { return m_mapHeight; }

	/// @brief タイル1個のピクセルサイズ
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

	// 内部関数：Tiled JSON をロード
	void loadFromJSON(const FilePath& jsonPath);
};

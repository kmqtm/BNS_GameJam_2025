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
	// 第3引数で「当たり判定として扱うレイヤー名」を受け取る
	Stage(const FilePath& json_path, const FilePath& tileset_path, const String& collision_layer_name);

	void Draw(const Vec2& camera_offset, const RectF& view_rect) const;

	// 指定したワールド座標 (ピクセル単位) が「壁」タイル上かどうかを判定する
	bool IsSolid(double world_x, double world_y) const;

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

	// --- 当たり判定用 ---
	String collision_layer_name_; // コンストラクタで受け取ったレイヤー名
	const TileMapLayer* collision_layer_ = nullptr; // 当たり判定レイヤーへのポインタ

	void LoadFromJson(const FilePath& json_path);
	void ParseTileLayer(const JSON& layer_json);
	void CreateTileRegions();
};

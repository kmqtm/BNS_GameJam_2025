#pragma once

/// @file Stage.h
/// @brief ステージ管理の定義

#include "SpawnInfo.h"
# include <Siv3D.hpp>

/// @brief タイルマップレイヤーを表現する構造体
struct TileMapLayer
{
	/// @brief レイヤー名
	String name;

	/// @brief タイルIDのグリッド
	Grid<int32> tiles;
};

/// @brief Tiledで作成されたマップを管理する
///
/// JSONファイルから読み込んだタイルマップの描画，衝突判定，
/// スポーン情報の管理を行う
class Stage
{
public:
	/// @brief コンストラクタ
	/// @param json_path マップデータのJSONファイルパス
	/// @param tileset_path タイルセット画像のファイルパス
	/// @param collision_layer_name 衝突判定用レイヤー名
	Stage(const FilePath& json_path, const FilePath& tileset_path, const String& collision_layer_name);

	/// @brief ステージを描画する
	/// @param camera_offset カメラオフセット
	/// @param view_rect ビュー矩形
	void Draw(const Vec2& camera_offset, const RectF& view_rect) const;

	/// @brief 指定したワールド座標が壁タイル上かを判定する
	/// @param world_x ワールド座標X
	/// @param world_y ワールド座標Y
	/// @return 壁タイル上の場合はtrue
	bool IsSolid(double world_x, double world_y) const;

	/// @brief スポーン情報のリストを取得する
	/// @return スポーン情報のリスト
	const s3d::Array<SpawnInfo>& GetSpawnPoints() const;

	/// @brief マップの幅(タイル数)を取得する
	/// @return マップの幅
	int32 GetWidth() const { return map_width_; }

	/// @brief マップの高さ(タイル数)を取得する
	/// @return マップの高さ
	int32 GetHeight() const { return map_height_; }

	/// @brief タイルサイズを取得する
	/// @return タイルサイズ(ピクセル)
	int32 GetTileSize() const { return tile_size_; }

private:
	/// @brief マップの幅(タイル数)
	int32 map_width_ = 0;

	/// @brief マップの高さ(タイル数)
	int32 map_height_ = 0;

	/// @brief タイルサイズ(ピクセル)
	int32 tile_size_ = 16;

	/// @brief すべてのタイルマップレイヤー
	Array<TileMapLayer> layers_;

	/// @brief タイルセットテクスチャ
	Texture tile_texture_;

	/// @brief タイルテクスチャリージョンのリスト
	Array<TextureRegion> tile_regions_;

	/// @brief スポーン情報のリスト
	Array<SpawnInfo> spawn_points_;

	/// @brief コンストラクタで受け取った衝突判定レイヤー名
	String collision_layer_name_;

	/// @brief 衝突判定レイヤーへのポインタ
	const TileMapLayer* collision_layer_ = nullptr;

	/// @brief JSONからマップデータを読み込む
	/// @param json_path JSONファイルパス
	void LoadFromJson(const FilePath& json_path);

	/// @brief タイルレイヤーを解析する
	/// @param layer_json タイルレイヤーのJSONデータ
	void ParseTileLayer(const JSON& layer_json);

	/// @brief オブジェクトレイヤーを解析する
	/// @param layer_json オブジェクトレイヤーのJSONデータ
	void ParseObjectLayer(const JSON& layer_json);

	/// @brief タイルテクスチャリージョンを生成する
	void CreateTileRegions();

	/// @brief 衝突判定レイヤーを探索する
	void FindCollisionLayer();

	/// @brief レイヤーのタイルを描画する
	/// @param layer 描画するレイヤー
	/// @param start_x 描画開始X(タイル座標)
	/// @param start_y 描画開始Y(タイル座標)
	/// @param end_x 描画終了X(タイル座標)
	/// @param end_y 描画終了Y(タイル座標)
	/// @param camera_offset カメラオフセット
	void DrawLayerTiles(const TileMapLayer& layer, int32 start_x, int32 start_y, int32 end_x, int32 end_y, const Vec2& camera_offset) const;

	/// @brief ビュー矩形から描画範囲を計算する
	/// @param view_rect ビュー矩形
	/// @param out_start_x 出力:描画開始X(タイル座標)
	/// @param out_start_y 出力:描画開始Y(タイル座標)
	/// @param out_end_x 出力:描画終了X(タイル座標)
	/// @param out_end_y 出力:描画終了Y(タイル座標)
	void ComputeDrawRange(const RectF& view_rect, int32& out_start_x, int32& out_start_y, int32& out_end_x, int32& out_end_y) const;
};

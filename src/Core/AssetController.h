#pragma once

#include <Siv3D.hpp>

// アセットの読み込み，登録，登録解除を管理するクラス
// シーンごとに必要なアセットをJSONファイルから読み込む
class AssetController {
public:
	static AssetController& GetInstance();

	// 指定されたシーン名に基づいてアセットを準備(登録・ロード)
	void PrepareAssets(const String &scene_name);

	// 現在のシーンで登録されているアセットの登録をすべて解除
	void UnregisterAssets();

	// コピーコンストラクタとコピー代入演算子を禁止
	AssetController(const AssetController&) = delete;
	AssetController& operator=(const AssetController&) = delete;

private:
	AssetController();

	// アセット情報を定義したJSONデータを保持
	JSON asset_json_;

	// 現在対象となっているシーン名
	String current_scene_name_;

	// 対象とするアセットの種類を定義した配列
	const Array<String> asset_types_ = {U"Font", U"Sound", U"Texture"};

	// アセットの登録とロードを行うヘルパー関数
	void RegisterAndLoadAsset(const String &asset_type, const String &asset_filepath);
};

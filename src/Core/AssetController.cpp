#include "AssetController.h"

#include <Siv3D.hpp>

// staticなGetInstance()メソッドの実装
AssetController& AssetController::GetInstance() {
	// この関数が初めて呼ばれたときに一度だけインスタンスが作成される
	static AssetController instance;
	return instance;
}

// privateなコンストラクタの実装 (中身は空でよい)
AssetController::AssetController() {
}

void AssetController::PrepareAssets(const String &scene_name) {
	// アセット情報のJSONを読み込む
	asset_json_ = JSON::Load(U"asset/AssetInfomation.json");
	if (!asset_json_) {
		Print << U"AssetInfomation.jsonの読み込みに失敗しました．";
		return;
	}

	current_scene_name_ = scene_name;

	// アセットのタイプごとに処理
	for (const auto &asset_type : asset_types_) {
		// アセットファイル名を1つずつ取得
		for (const auto &file_name_json : asset_json_[current_scene_name_][asset_type]) {
			const String asset_file_name = file_name_json.value.getString();

			// アセット名が "null" ならスキップ
			if (asset_file_name == U"null") {
				continue;
			}

			// アセットのファイルパスを構築
			const String asset_filepath = U"asset/" + asset_type + U"/" + asset_file_name;

			// タイプ別のフォルダ内に対象のアセットファイルが存在するか確認
			if (FileSystem::Exists(asset_filepath)) {
				// アセットの登録とロード
				RegisterAndLoadAsset(asset_type, asset_filepath);
			}
		}
	}
}

void AssetController::UnregisterAssets() {
	if (!asset_json_) {
		// JSONがロードされていない場合は何もしない
		return;
	}

	// アセットのタイプごとに処理
	for (const auto &asset_type : asset_types_) {
		// アセットファイル名を1つずつ取得
		for (const auto &file_name_json : asset_json_[current_scene_name_][asset_type]) {
			if (JSONValueType::Empty == file_name_json.value.getType()) {
				Console << U"empty";
				continue;
			}
			const String asset_file_name = file_name_json.value.getString();

			// アセット名が "null" ならスキップ
			if (asset_file_name == U"null") continue;

			const String asset_base_name = FileSystem::BaseName(asset_file_name);

			if (asset_type == U"Font") {
				FontAsset::Unregister(asset_base_name);
			} else if (asset_type == U"Sound") {
				AudioAsset::Unregister(asset_base_name);
			} else if (asset_type == U"Texture") {
				TextureAsset::Unregister(asset_base_name);
			}
		}
	}
}

void AssetController::RegisterAndLoadAsset(const String &asset_type, const String &asset_filepath) {
	// 拡張子を除くファイル名
	const String asset_base_name = FileSystem::BaseName(asset_filepath);

	// タイプ別で処理
	if (asset_type == U"Font") {
		const int32 font_size = asset_json_[current_scene_name_][U"FontSize"].get<int32>();
		FontAsset::Register(asset_base_name, font_size, asset_filepath);
		FontAsset::Load(asset_base_name);
	} else if (asset_type == U"Sound") {
		AudioAsset::Register(asset_base_name, asset_filepath);
		AudioAsset::Load(asset_base_name);
	} else if (asset_type == U"Texture") {
		TextureAsset::Register(asset_base_name, asset_filepath);
		TextureAsset::Load(asset_base_name);
	}
}

#pragma once

/// @file AssetSystem.h
/// @brief アセット管理システムの定義

#include <chrono>
#include <Siv3D.hpp>
#include <thread>
#include <utility>
#include <vector>

/// @brief アセット読み込みと登録を管理するシングルトン
///
/// シーンごとに必要なアセットをJSONファイルから読み込み，同期・非同期での
/// 登録・ロードを制御する
class AssetSystem
{
public:
	/// @brief シングルトンインスタンスを取得する
	/// @return AssetSystemのインスタンス
	static AssetSystem& GetInstance();

	/// @brief アセットのロードモード
	enum class LoadMode
	{
		Auto,   ///< 自動判別(デフォルト)
		Sync,   ///< 同期ロード
		Async   ///< 非同期ロード
	};

	/// @brief 指定したシーン名のアセットを準備する
	/// @param scene_name シーン名
	void PrepareAssets(const String& scene_name);

	/// @brief 現在のシーンのアセット登録をすべて解除する
	void UnregisterAssets();

	/// @brief 現在のシーンのアセット非同期ロードが完了したか
	/// @return 完了している場合はtrue
	bool IsSceneAssetsReady();

	/// @brief 非同期ロードが完了するまで待機する
	void WaitUntilReady();

	/// @brief コピーコンストラクタ(削除)
	AssetSystem(const AssetSystem&) = delete;

	/// @brief コピー代入演算子(削除)
	AssetSystem& operator=(const AssetSystem&) = delete;

private:
	/// @brief コンストラクタ
	AssetSystem();

	/// @brief アセット情報を定義したJSONデータ
	JSON asset_json_;

	/// @brief 現在対象となっているシーン名
	String current_scene_name_;

	/// @brief 対象とするアセットの種類
	const Array<String> asset_types_ = { U"Font", U"Sound", U"Texture" };

	/// @brief アセットを登録してロードする
	/// @param asset_type アセットの種類
	/// @param asset_filepath アセットのファイルパス
	/// @param mode ロードモード
	void RegisterAndLoadAsset(const String& asset_type, const String& asset_filepath, LoadMode mode);

	/// @brief 登録されたアセットの一覧(種類,基本名)
	Array<std::pair<String, String>> registered_assets_;

	/// @brief 非同期ロード中のアセット一覧(種類,基本名)
	Array<std::pair<String, String>> pending_assets_;
};

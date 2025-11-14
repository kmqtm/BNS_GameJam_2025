#include "AssetSystem.h"

#include <Siv3D.hpp>

AssetSystem& AssetSystem::GetInstance()
{
	static AssetSystem instance;
	return instance;
}

// JSONの読み込みは最初の一回だけ行う
AssetSystem::AssetSystem()
{
	asset_json_ = JSON::Load(U"asset/AssetInformation.json");
	if(!asset_json_)
	{
		throw std::runtime_error("AssetInformation.jsonの読み込みに失敗しました．");
	}
}

static AssetSystem::LoadMode ParseLoadModeFromJson(const JSON& j)
{
	if(j.isString())
	{
		return AssetSystem::LoadMode::Auto;
	}
	else if(j.isObject())
	{
		if(j.hasElement(U"loadMode") && j[U"loadMode"].isString())
		{
			const String mode = j[U"loadMode"].getString();
			if(mode == U"Sync") return AssetSystem::LoadMode::Sync;
			if(mode == U"Async") return AssetSystem::LoadMode::Async;
		}
	}
	return AssetSystem::LoadMode::Auto;
}

// Extract the asset file name (or "path") from a JSON entry which may be a string or an object.
static String ParseAssetFileNameFromJson(const JSON& entry)
{
	if(entry.isString())
	{
		return entry.getString();
	}
	if(entry.isObject())
	{
		if(entry.hasElement(U"path") && entry[U"path"].isString())
		{
			return entry[U"path"].getString();
		}
	}
	return String();
}

void AssetSystem::PrepareAssets(const String& scene_name)
{
	// コンストラクタで読み込み失敗した場合は即時リターン
	if(!asset_json_)
	{
		return;
	}

	if((not current_scene_name_.isEmpty()) && (current_scene_name_ != scene_name))
	{
		//先に古いシーンのアセットを解放する
		UnregisterAssets();
	}

	current_scene_name_ = scene_name;

	for(const auto& asset_type : asset_types_)
	{
		for(const auto& file_name_json : asset_json_[current_scene_name_][asset_type])
		{
			// parse load mode and file name in a helper to reduce nesting
			AssetSystem::LoadMode mode = ParseLoadModeFromJson(file_name_json.value);

			const String asset_file_name = ParseAssetFileNameFromJson(file_name_json.value);
			if(asset_file_name.isEmpty())
			{
				continue;
			}

			if(asset_file_name == U"null")
			{
				continue;
			}

			const String asset_filepath = U"asset/" + asset_type + U"/" + asset_file_name;

			if(FileSystem::Exists(asset_filepath))
			{
				RegisterAndLoadAsset(asset_type, asset_filepath, mode);
			}
		}
	}
}

void AssetSystem::UnregisterAssets()
{
	if(!asset_json_)
	{
		return;
	}

	// 非同期読み込みが終わるまで待つ
	WaitUntilReady();

	// 登録したものだけを解除する
	for(const auto& item : registered_assets_)
	{
		const String& asset_type = item.first;
		const String& asset_base_name = item.second;

		if(asset_type == U"Font")
		{
			if(FontAsset::IsRegistered(asset_base_name))
			{
				FontAsset::Unregister(asset_base_name);
			}
		}
		else if(asset_type == U"Sound")
		{
			if(AudioAsset::IsRegistered(asset_base_name))
			{
				AudioAsset::Unregister(asset_base_name);
			}
		}
		else if(asset_type == U"Texture")
		{
			if(TextureAsset::IsRegistered(asset_base_name))
			{
				TextureAsset::Unregister(asset_base_name);
			}
		}
	}

	registered_assets_.clear();
}

bool AssetSystem::IsSceneAssetsReady()
{
	// より読みやすく: pending_assets_ を走査して未完了のみを残す
	Array<std::pair<String, String>> remaining;
	remaining.reserve(pending_assets_.size());

	for(const auto& p : pending_assets_)
	{
		const String& type = p.first;
		const String& baseName = p.second;

		bool ready = true;

		if(type == U"Texture")
		{
			ready = TextureAsset::IsReady(baseName);
		}
		else if(type == U"Sound")
		{
			ready = AudioAsset::IsReady(baseName);
		}
		else if(type == U"Font")
		{
			ready = FontAsset::IsReady(baseName);
		}

		if(!ready)
		{
			remaining.push_back(p);
		}
	}

	pending_assets_.swap(remaining);
	return pending_assets_.isEmpty();
}

void AssetSystem::WaitUntilReady()
{
	while(!IsSceneAssetsReady())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void AssetSystem::RegisterAndLoadAsset(const String& asset_type, const String& asset_filepath, AssetSystem::LoadMode mode)
{
	const String asset_base_name = FileSystem::BaseName(asset_filepath);

	// Determine mode if Auto
	if(mode == AssetSystem::LoadMode::Auto)
	{
		// Simple heuristic: textures and sounds => Async, fonts => Sync
		if(asset_type == U"Texture" || asset_type == U"Sound")
		{
			mode = AssetSystem::LoadMode::Async;
		}
		else
		{
			mode = AssetSystem::LoadMode::Sync;
		}
	}

	// small lambda to centralize registered_assets_ push
	auto markRegistered = [&](const String& type, const String& name)
		{
			registered_assets_.push_back({ type, name });
		};

	if(asset_type == U"Font")
	{
		int32 font_size = 24; // default
		// JSON に FontSize があれば取得
		if(JSONValueType::Empty != asset_json_[current_scene_name_][U"FontSize"].getType())
		{
			font_size = asset_json_[current_scene_name_][U"FontSize"].get<int32>();
		}

		FontAsset::Register(asset_base_name, font_size, asset_filepath);

		if(mode == AssetSystem::LoadMode::Async)
		{
			FontAsset::LoadAsync(asset_base_name);
			pending_assets_.push_back({ asset_type, asset_base_name });
		}
		else
		{
			FontAsset::Load(asset_base_name);
		}

		markRegistered(asset_type, asset_base_name);
	}
	else if(asset_type == U"Sound")
	{
		AudioAsset::Register(asset_base_name, asset_filepath);

		if(mode == AssetSystem::LoadMode::Async)
		{
			AudioAsset::LoadAsync(asset_base_name);
			pending_assets_.push_back({ asset_type, asset_base_name });
		}
		else
		{
			AudioAsset::Load(asset_base_name);
		}

		markRegistered(asset_type, asset_base_name);
	}
	else if(asset_type == U"Texture")
	{
		TextureAsset::Register(asset_base_name, asset_filepath);

		if(mode == AssetSystem::LoadMode::Async)
		{
			TextureAsset::LoadAsync(asset_base_name);
			pending_assets_.push_back({ asset_type, asset_base_name });
		}
		else
		{
			TextureAsset::Load(asset_base_name);
		}

		markRegistered(asset_type, asset_base_name);
	}
}

#include "AssetController.h"

#include <Siv3D.hpp>

AssetController& AssetController::GetInstance()
{
	static AssetController instance;
	return instance;
}

// JSONの読み込みは最初の一回だけ行う
AssetController::AssetController()
{
	asset_json_ = JSON::Load(U"asset/AssetInfomation.json");
	if(!asset_json_)
	{
		throw std::runtime_error("AssetInfomation.jsonの読み込みに失敗しました．");
	}
}

void AssetController::PrepareAssets(const String& scene_name)
{
	// コンストラクタで読み込み失敗した場合は即時リターン
	if(!asset_json_)
	{
		return;
	}

	if((not current_scene_name_.isEmpty()) && (current_scene_name_ != scene_name))
	{
		// 先に古いシーンのアセットを解放する
		UnregisterAssets();
	}

	current_scene_name_ = scene_name;

	for(const auto& asset_type : asset_types_)
	{
		for(const auto& file_name_json : asset_json_[current_scene_name_][asset_type])
		{
			const String asset_file_name = file_name_json.value.getString();

			if(asset_file_name == U"null")
			{
				continue;
			}

			const String asset_filepath = U"asset/" + asset_type + U"/" + asset_file_name;

			if(FileSystem::Exists(asset_filepath))
			{
				RegisterAndLoadAsset(asset_type, asset_filepath);
			}
		}
	}
}

void AssetController::UnregisterAssets()
{
	if(!asset_json_)
	{
		return;
	}

	for(const auto& asset_type : asset_types_)
	{
		for(const auto& file_name_json : asset_json_[current_scene_name_][asset_type])
		{
			if(JSONValueType::Empty == file_name_json.value.getType())
			{
				Console << U"empty";
				continue;
			}
			const String asset_file_name = file_name_json.value.getString();

			if(asset_file_name == U"null") continue;

			const String asset_base_name = FileSystem::BaseName(asset_file_name);

			if(asset_type == U"Font")
			{
				FontAsset::Unregister(asset_base_name);
			}
			else if(asset_type == U"Sound")
			{
				AudioAsset::Unregister(asset_base_name);
			}
			else if(asset_type == U"Texture")
			{
				TextureAsset::Unregister(asset_base_name);
			}
		}
	}
}

void AssetController::RegisterAndLoadAsset(const String& asset_type, const String& asset_filepath)
{
	const String asset_base_name = FileSystem::BaseName(asset_filepath);

	if(asset_type == U"Font")
	{
		const int32 font_size = asset_json_[current_scene_name_][U"FontSize"].get<int32>();
		FontAsset::Register(asset_base_name, font_size, asset_filepath);
		FontAsset::Load(asset_base_name);
	}
	else if(asset_type == U"Sound")
	{
		AudioAsset::Register(asset_base_name, asset_filepath);
		AudioAsset::Load(asset_base_name);
	}
	else if(asset_type == U"Texture")
	{
		TextureAsset::Register(asset_base_name, asset_filepath);
		TextureAsset::Load(asset_base_name);
	}
}

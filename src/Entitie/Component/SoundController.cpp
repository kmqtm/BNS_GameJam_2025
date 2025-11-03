#include "SoundController.h"

SoundController::SoundController()
{
}

void SoundController::Play(const String& asset_name, bool loop)
{
	if(not AudioAsset::IsRegistered(asset_name))
	{
		Print << U"エラー: 音声アセット'{}'は登録されていません．"_fmt(asset_name);
		return;
	}

	// 登録はされているが準備(ロード)が完了していない場合は再生しない（非同期ロードの完了を待つ）
	if(not AudioAsset::IsReady(asset_name))
	{
		// 非ブロッキング:まだロードされていないので再生リクエストを無視
		return;
	}

	//既に再生中の場合は停止してから再生
	if(AudioAsset(asset_name).isPlaying())
	{
		AudioAsset(asset_name).stop();
	}

	AudioAsset(asset_name).setLoop(loop);
	AudioAsset(asset_name).play();
}

void SoundController::Stop(const String& asset_name)
{
	if(AudioAsset::IsRegistered(asset_name))
	{
		AudioAsset(asset_name).stop();
	}
}

void SoundController::StopAll()
{
	const auto assets = AudioAsset::Enumerate();
	for(const auto& [name, info] : assets)
	{
		AudioAsset(name).stop();
	}
}

bool SoundController::IsPlaying(const String& asset_name) const
{
	if(not AudioAsset::IsRegistered(asset_name))
	{
		return false;
	}

	return AudioAsset(asset_name).isPlaying();
}

void SoundController::SetVolume(const String& asset_name, double volume)
{
	if(AudioAsset::IsRegistered(asset_name))
	{
		AudioAsset(asset_name).setVolume(volume);
	}
}

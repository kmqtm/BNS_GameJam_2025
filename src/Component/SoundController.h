#pragma once

/// @file SoundController.h
/// @brief サウンド再生管理クラス

#include <Siv3D.hpp>

/// @brief サウンド再生を管理する
class SoundController
{
public:
	/// @brief コンストラクタ
	SoundController();

	/// @brief サウンドを再生する
	/// @param asset_name サウンドのアセット名
	/// @param loop ループ再生するか(デフォルト:false)
	void Play(const String& asset_name, bool loop = false);

	/// @brief 指定したサウンドを停止する
	/// @param asset_name 停止するサウンドのアセット名
	void Stop(const String& asset_name);

	/// @brief 再生中のすべてのサウンドを停止する
	void StopAll();

	/// @brief 指定したサウンドが再生中かを判定する
	/// @param asset_name 確認するサウンドのアセット名
	/// @return 再生中の場合はtrue，それ以外の場合はfalse
	[[nodiscard]]
	bool IsPlaying(const String& asset_name) const;

	/// @brief 指定したサウンドの音量を設定する
	/// @param asset_name サウンドのアセット名
	/// @param volume 設定する音量(0.0～1.0の範囲)
	void SetVolume(const String& asset_name, double volume);
};

#pragma once

/// @file Animation.h
/// @brief アニメーションパターンの定義

#include <Siv3D.hpp>

/// @brief アニメーション再生に必要な情報を保持する構造体
struct Animation
{
	/// @brief アニメーションを構成するテクスチャアセット名のリスト
	Array<String> texture_asset_names;

	/// @brief 1フレームあたりの表示時間(秒)
	double frame_duration_sec;

	/// @brief アニメーションをループ再生するか
	bool is_looping;
};

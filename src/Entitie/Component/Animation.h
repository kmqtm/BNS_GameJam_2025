#pragma once

#include <Siv3D.hpp>

// 1つのアニメーションパターンを定義する構造体
struct Animation
{
	// アニメーションを構成するテクスチャアセット名のリスト
	Array<String> texture_asset_names;

	// 1フレームあたりの表示時間（秒）
	// 0.2 に設定すると，0.2秒ごとに次の画像へ切り替わる
	double frame_duration_sec;

	// アニメーションをループ再生するか
	bool is_looping;
};

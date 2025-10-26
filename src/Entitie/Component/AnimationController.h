#pragma once

#include "Animation.h"

#include <Siv3D.hpp>

class AnimationController {
public:
	AnimationController();

	// アニメーションパターンを名前付きで登録
	void AddAnimation(const String &name, const Animation &animation);

	// 指定した名前のアニメーションの再生を開始
	void Play(const String &name);

	// 特定のアニメーションが再生中かどうかを取得
	bool IsPlaying(const String& animation_name) const;

	// 毎フレーム呼び出し，アニメーションの状態を更新
	void Update();

	// 現在表示すべきテクスチャアセットを取得
	s3d::Optional<TextureAsset> GetCurrentTextureAsset() const;

private:
	// 登録された全アニメーションパターンを保持
	HashTable<String, Animation> animations_;

	// 現在再生中のアニメーションの名前
	String current_animation_name_;

	// 現在のフレーム番号
	size_t current_frame_index_;

	// フレーム切り替え用のタイマー
	Stopwatch frame_timer_;
};

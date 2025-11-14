#pragma once

#include "Animation.h"

#include <Siv3D.hpp>

class AnimationController
{
public:
	AnimationController();

	void AddAnimation(const String& name, const Animation& animation);
	void Play(const String& name);
	bool IsPlaying(const String& animation_name) const;

	void Update();

	s3d::Optional<TextureAsset> GetCurrentTextureAsset() const;

private:
	HashTable<String, Animation> animations_;
	String current_animation_name_;
	size_t current_frame_index_;
	Stopwatch frame_timer_;

	// Update() / GetCurrentTextureAsset() での毎フレームの検索を避けるため，
	// 現在のアニメーションデータへのポインタをキャッシュ
	const Animation* current_animation_ = nullptr;
};
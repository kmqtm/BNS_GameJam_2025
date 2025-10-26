#include "AnimationController.h"

AnimationController::AnimationController()
	: current_frame_index_{ 0 }
	, current_animation_{ nullptr } // (リファクタリング) ポインタを nullptr で初期化
{
}

void AnimationController::AddAnimation(const String& name, const Animation& animation)
{
	animations_[name] = animation;
}

void AnimationController::Play(const String& name)
{
	if(not animations_.contains(name))
	{
		return;
	}

	if(current_animation_name_ == name)
	{
		return;
	}

	current_animation_name_ = name;
	current_frame_index_ = 0;
	frame_timer_.restart();

	// アニメーションデータへのポインタをキャッシュ
	current_animation_ = &animations_.at(name);
}

bool AnimationController::IsPlaying(const String& animation_name) const
{
	return current_animation_name_ == animation_name;
}

void AnimationController::Update()
{
	if(not current_animation_)
	{
		return;
	}

	if(frame_timer_.sF() < current_animation_->frame_duration_sec)
	{
		return;
	}

	current_frame_index_++;
	frame_timer_.restart();

	if(current_frame_index_ >= current_animation_->texture_asset_names.size())
	{
		if(current_animation_->is_looping)
		{
			current_frame_index_ = 0;
		}
		else
		{
			current_frame_index_ = current_animation_->texture_asset_names.size() - 1;
		}
	}
}

s3d::Optional<TextureAsset> AnimationController::GetCurrentTextureAsset() const
{
	if(not current_animation_)
	{
		return s3d::none;
	}

	const String& asset_name = current_animation_->texture_asset_names[current_frame_index_];

	if(not TextureAsset::IsRegistered(asset_name))
	{
		Print << U"エラー: アセット名'{}'は登録されていません．"_fmt(asset_name);
		return s3d::none;
	}

	return TextureAsset(asset_name);
}
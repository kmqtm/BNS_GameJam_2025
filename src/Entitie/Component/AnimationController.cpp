#include "AnimationController.h"

AnimationController::AnimationController() : current_frame_index_{ 0 }
{
	// メンバ変数の初期化
}

void AnimationController::AddAnimation(const String& name, const Animation& animation)
{
	// "walk"などの名前とアニメーションデータを紐づけてハッシュテーブルに登録
	animations_[name] = animation;
}

void AnimationController::Play(const String& name)
{
	// 再生したいアニメーション名が登録されているか確認
	if(not animations_.contains(name))
	{
		// 登録されていなければ何もしない
		return;
	}

	// もし既に同じアニメーションを再生中なら，何もせず終了
	// これにより，毎フレームPlayを呼んでもアニメーションが1フレーム目に戻るのを防ぐ
	if(current_animation_name_ == name)
	{
		return;
	}

	// 再生するアニメーションを切り替える
	current_animation_name_ = name;
	// フレームを最初のコマに戻す
	current_frame_index_ = 0;
	// フレーム切り替え用のタイマーをリスタートさせる
	frame_timer_.restart();
}

bool AnimationController::IsPlaying(const String& animation_name) const
{
	// 現在再生中のアニメーション名と引数の名前が同じかを返す
	return current_animation_name_ == animation_name;
}

void AnimationController::Update()
{
	// 再生中のアニメーションがなければ，何もしない
	if(current_animation_name_.isEmpty() || (not animations_.contains(current_animation_name_)))
	{
		return;
	}

	// 現在再生中のアニメーションデータを取得
	const auto& current_anim = animations_.at(current_animation_name_);

	// 前回のフレーム切り替えから，1フレームの表示時間を超えていない場合は何もしない
	if(frame_timer_.sF() < current_anim.frame_duration_sec)
	{
		return;
	}

	// --- 1フレームの表示時間を超えた場合，次のフレームへ ---

	// フレーム番号を1つ進める
	current_frame_index_++;
	// タイマーをリスタートして，次のフレーム時間を計測し始める
	frame_timer_.restart();

	// 現在のフレーム番号が，アニメーションの総フレーム数を超えたかチェック
	if(current_frame_index_ >= current_anim.texture_asset_names.size())
	{
		// 超えた場合
		if(current_anim.is_looping)
		{
			// ループ再生が有効なら，フレーム番号を最初(0)に戻す
			current_frame_index_ = 0;
		}
		else
		{
			// ループ再生が無効なら，フレーム番号を最後で止める
			current_frame_index_ = current_anim.texture_asset_names.size() - 1;
		}
	}
}

s3d::Optional<TextureAsset> AnimationController::GetCurrentTextureAsset() const
{
	// 再生中のアニメーションがなければ，空のテクスチャアセットを返す
	if(current_animation_name_.isEmpty() || (not animations_.contains(current_animation_name_)))
	{
		return s3d::none;
	}

	// 現在のアニメーションデータとフレーム番号から，表示すべきテクスチャのアセット名を取得
	const auto& current_anim = animations_.at(current_animation_name_);
	const String& asset_name = current_anim.texture_asset_names[current_frame_index_];

	if(not TextureAsset::IsRegistered(asset_name))
	{
		// アセット名が登録されていなかった場合も「空」を返す
		Print << U"エラー: アセット名'{}'は登録されていません．"_fmt(asset_name);
		return s3d::none;
	}

	// アセット名からTextureAssetを返す
	return TextureAsset(asset_name);
}

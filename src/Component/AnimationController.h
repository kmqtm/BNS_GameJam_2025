#pragma once

/// @file AnimationController.h
/// @brief アニメーション再生管理クラス

#include "Animation.h"

#include <Siv3D.hpp>

/// @brief アニメーション再生を管理する
class AnimationController
{
public:
	/// @brief コンストラクタ
	AnimationController();

	/// @brief アニメーションを登録する
	/// @param name 識別名
	/// @param animation アニメーションデータ
	void AddAnimation(const String& name, const Animation& animation);

	/// @brief 指定したアニメーションの再生を開始する
	/// @param name 再生するアニメーションの識別名
	void Play(const String& name);

	/// @brief 指定したアニメーションが再生中かを判定する
	/// @param animation_name 確認するアニメーションの識別名
	/// @return 再生中の場合はtrue，それ以外の場合はfalse
	bool IsPlaying(const String& animation_name) const;

	/// @brief アニメーションを1フレーム進める
	void Update();

	/// @brief 現在のフレームのテクスチャを取得する
	/// @return テクスチャアセット。再生中でない場合はnullopt
	s3d::Optional<TextureAsset> GetCurrentTextureAsset() const;

private:
	/// @brief 登録されたアニメーション(識別名とアニメーションデータのマッピング)
	HashTable<String, Animation> animations_;

	/// @brief 現在再生中のアニメーションの識別名
	String current_animation_name_;

	/// @brief 現在再生中のフレームインデックス
	size_t current_frame_index_;

	/// @brief フレーム表示時間を計測するストップウォッチ
	Stopwatch frame_timer_;

	/// @brief 現在のアニメーションデータへのキャッシュポインタ
	const Animation* current_animation_ = nullptr;
};

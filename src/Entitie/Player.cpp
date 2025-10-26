#include "../Core/Config.h"
#include "../Core/Utility.h"
#include "Component/Animation.h"
#include "Player.h"

#include <Siv3D.hpp>

Player::Player()
{
	// アニメーションの初期化処理

	// 「歩き (泳ぎ)」アニメーションのデータを定義
	Animation walk_animation;
	walk_animation.texture_asset_names = {
		U"player_2", // アセット名 "player_2"
		U"player_1", // アセット名 "player_1"
	};
	walk_animation.frame_duration_sec = 0.08;
	walk_animation.is_looping = true;
	anim_controller_.AddAnimation(U"walk", walk_animation);

	// 「待機」アニメーションのデータを定義
	Animation idle_animation;
	idle_animation.texture_asset_names = { U"player_1" };
	idle_animation.frame_duration_sec = 1.0;
	idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"idle", idle_animation);

	// 「ひと泳ぎ」アニメーションのデータを定義
	Animation swim_animation;
	swim_animation.texture_asset_names = { U"player_2" }; // 既存のアセットを流用
	swim_animation.frame_duration_sec = 0.2;            // 0.2秒間だけこのポーズ
	swim_animation.is_looping = false;                  // ループしない
	anim_controller_.AddAnimation(U"swim", swim_animation);
}

void Player::Update()
{
	// --- 1. 入力処理 (左右) ---
	bool is_moving_x = false;
	if(kInputLeft.pressed())
	{
		// 左に加速 (最大速度まで)
		velocity_.x = Max(velocity_.x - horizontal_accel_, -horizontal_speed_max_);
		is_moving_x = true;
	}
	else if(kInputRight.pressed())
	{
		// 右に加速 (最大速度まで)
		velocity_.x = Min(velocity_.x + horizontal_accel_, horizontal_speed_max_);
		is_moving_x = true;
	}
	else
	{
		// 水平方向の抵抗（摩擦）
		velocity_.x *= friction_;
		// 速度が非常に小さくなったらゼロにする
		if(std::abs(velocity_.x) < 0.1)
		{
			velocity_.x = 0.0;
		}
	}

	// --- 2. 入力処理 (浮上) ---
	// スペースキーが押された「瞬間」
	if(KeySpace.down())
	{
		velocity_.y = swim_power_;      // 上向きの初速を与える
		anim_controller_.Play(U"swim"); // 泳ぎアニメーションを再生
	}

	// --- 3. 物理演算 (Y軸) ---
	// プレイヤーが上昇中 (velocity_.y < 0) かどうかで重力を変える
	if(velocity_.y < 0)
	{
		// 上昇中は重力の効きを弱める
		velocity_.y += (gravity_ * rising_gravity_multiplier_);
	}
	else
	{
		// 落下中 (velocity_.y >= 0) は通常の重力を適用
		velocity_.y += gravity_;
	}

	// 最大落下速度を超えないようにする
	velocity_.y = Min(velocity_.y, terminal_velocity_y_);

	// --- 4. 位置更新 ---
	pos_ += velocity_; // 速度を位置に反映

	// --- 5. アニメーション制御 ---
	// "swim" アニメーションが再生中ではない場合のみ、他のアニメーションを制御
	if(not anim_controller_.IsPlaying(U"swim"))
	{
		if(is_moving_x)
		{
			anim_controller_.Play(U"walk");
		}
		else
		{
			anim_controller_.Play(U"idle");
		}
	}

	// アニメーションコントローラを更新
	anim_controller_.Update();
}

void Player::Draw() const
{
	// GetCurrentTextureAssetから値を受け取る
	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		// ピクセルアートがぼやけないよう、座標を整数に丸めて描画
		texture_asset->draw(Utility::RoundVec2(pos_));
	}
	else
	{
		// 中身がなかった場合(エラー時)の処理
	}
}

Vec2 Player::GetPos() const { return pos_; }

void Player::SetPos(const Vec2& new_pos)
{
	pos_ = new_pos;
	velocity_ = Vec2::Zero(); // 位置がリセットされたら速度もリセット
}

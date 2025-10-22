#include "Player.h"

#include "../Core/Config.h"

#include <Siv3D.hpp>
#include <cmath>

Player::Player(){
	// --- アニメーションの初期化処理 ---
	// 「歩き」アニメーションのデータを定義
	Animation walk_animation;
	walk_animation.texture_asset_names = {
		U"player_2",  // アセット名 "player_2"
		U"player_1",  // アセット名 "player_1"
	};
	walk_animation.frame_duration_sec = 0.08;  // 絵を切り替える秒数
	walk_animation.is_looping = true;

	// AnimationControllerに"walk"という名前で登録
	anim_controller_.AddAnimation(U"walk", walk_animation);

	// 「待機」アニメーションのデータを定義
	Animation idle_animation;
	idle_animation.texture_asset_names = {U"player_1"};
	idle_animation.frame_duration_sec = 1.0;  // ずっと1枚絵なので時間は任意
	idle_animation.is_looping = false;

	// "idle"という名前で登録
	anim_controller_.AddAnimation(U"idle", idle_animation);
}

void Player::Update() {
	bool is_moving = false;
	if (kInputUp.pressed() || kInputDown.pressed() || kInputLeft.pressed() || kInputRight.pressed()) {
		if (kInputLeft.pressed()) {
			pos_.x -= speed_;
		}
		if (kInputRight.pressed()) {
			pos_.x += speed_;
		}

		is_moving = true;
	}

	// 状態に応じて再生するアニメーションを決定
	if (is_moving) {
		anim_controller_.Play(U"walk");
	} else {
		anim_controller_.Play(U"idle");
	}

	// アニメーションコントローラを更新
	anim_controller_.Update();
}

void Player::Draw() const {
	// GetCurrentTextureAssetから値を受け取る
	if (auto texture_asset = anim_controller_.GetCurrentTextureAsset()) {
		// 中身があった場合のみ描画する
		texture_asset->draw(pos_.x, pos_.y);
	} else {
		// 中身がなかった場合(エラー時)の処理
		// ここでデバッグ情報を表示したり，何もしなかったりできる
	}
}

Vec2 Player::GetPos() const {
	return pos_;
}

void Player::SetPos(const Vec2 &new_pos) {
	pos_ = new_pos;
}

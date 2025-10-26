#include "../Core/Config.h"
#include "../Core/Utility.h"
#include "Component/Animation.h"
#include "Player.h"

#include <Siv3D.hpp>

Player::Player()
{
	// アニメーションの初期化処理
	Animation floating_move_animation;
	floating_move_animation.texture_asset_names = { U"player_4", U"player_5", U"player_6" };
	floating_move_animation.frame_duration_sec = 0.18;
	floating_move_animation.is_looping = true;
	anim_controller_.AddAnimation(U"floating_move", floating_move_animation);

	Animation idle_animation;
	idle_animation.texture_asset_names = { U"player_1" };
	idle_animation.frame_duration_sec = 1.0;
	idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"idle", idle_animation);

	Animation swim_animation;
	swim_animation.texture_asset_names = { U"player_2", U"player_3" };
	swim_animation.frame_duration_sec = 0.2;
	swim_animation.is_looping = false;
	anim_controller_.AddAnimation(U"swim", swim_animation);
}

void Player::Update()
{
	HandleInput();
	UpdatePhysics();
	UpdateAnimation();

	anim_controller_.Update();
}

// 入力処理
void Player::HandleInput()
{
	// 左右移動
	is_moving_x_ = false;
	if(kInputLeft.pressed())
	{
		velocity_.x = Max(velocity_.x - horizontal_accel_, -horizontal_speed_max_);
		is_moving_x_ = true;
	}
	else if(kInputRight.pressed())
	{
		velocity_.x = Min(velocity_.x + horizontal_accel_, horizontal_speed_max_);
		is_moving_x_ = true;
	}
	else
	{
		// 水平方向の抵抗
		velocity_.x *= friction_;
		if(std::abs(velocity_.x) < 0.1)
		{
			velocity_.x = 0.0;
		}
	}

	// 泳ぎ
	if(KeySpace.down())
	{
		velocity_.y = swim_power_;

		anim_controller_.Play(U"idle");
		anim_controller_.Play(U"swim");
	}
}

// 物理演算と位置更新
void Player::UpdatePhysics()
{
	// Y軸方向の重力処理
	if(velocity_.y < 0)
	{
		velocity_.y += (gravity_ * rising_gravity_multiplier_);
	}
	else
	{
		velocity_.y += gravity_;
	}

	velocity_.y = Min(velocity_.y, terminal_velocity_y_);

	// 位置更新
	pos_ += velocity_;
}

// アニメーション制御
void Player::UpdateAnimation()
{
	// swimアニメーションが再生中(または終了してスタック中)か確認
	if(anim_controller_.IsPlaying(U"swim"))
	{
		// 泳ぎによる上昇が終わり，重力で落下し始めたか確認
		if(velocity_.y > 0)
		{
			// swimの物理動作が終わっているので，移動状態かアイドル状態に強制遷移させる
			if(is_moving_x_)
			{
				anim_controller_.Play(U"floating_move");
			}
			else
			{
				anim_controller_.Play(U"idle");
			}
		}
	}
	else
	{
		// swimが再生中でない(ゲーム開始時など)場合は，通常通り
		if(is_moving_x_)
		{
			anim_controller_.Play(U"floating_move");
		}
		else
		{
			anim_controller_.Play(U"idle");
		}
	}
}

void Player::Draw(const Vec2& camera_offset) const
{
	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		// プレイヤーのワールド座標を整数にスナップ
		const Vec2 snapped_pos = Utility::RoundVec2(pos_);
		// スクリーン座標 = ワールド座標 - カメラオフセット
		const Vec2 draw_pos = snapped_pos - camera_offset;

		texture_asset->draw(draw_pos);
	}
	else
	{
		// texture_assetの中身がなかった場合
		RectF{ pos_ - Vec2{16, 16}, 32, 32 }.drawFrame(2, 0, Palette::Red);
	}
}

Vec2 Player::GetPos() const { return pos_; }

void Player::SetPos(const Vec2& new_pos)
{
	pos_ = new_pos;
	velocity_ = Vec2::Zero();
}
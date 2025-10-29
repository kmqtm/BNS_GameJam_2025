#include "../Core/Config.h"
#include "../Core/Utility.h"
#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Player.h"

#include <Siv3D.hpp>

Player::Player()
{
	// アニメーションの初期化処理
	Animation ground_idle_animation;
	ground_idle_animation.texture_asset_names = { U"player_stand" };
	ground_idle_animation.frame_duration_sec = 1.0;
	ground_idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"ground_idle", ground_idle_animation);

	Animation float_idle_animation;
	float_idle_animation.texture_asset_names = { U"player_1" };
	float_idle_animation.frame_duration_sec = 1.0;
	float_idle_animation.is_looping = false;
	anim_controller_.AddAnimation(U"float_idle", float_idle_animation);

	Animation walk_animation;
	walk_animation.texture_asset_names = { U"player_walk1", U"player_walk2", U"player_walk3", U"player_walk4", U"player_walk5", U"player_walk6", };
	walk_animation.frame_duration_sec = 0.32;
	walk_animation.is_looping = true;
	anim_controller_.AddAnimation(U"walk", walk_animation);

	Animation float_move_animation;
	float_move_animation.texture_asset_names = { U"player_4", U"player_5", U"player_6" };
	float_move_animation.frame_duration_sec = 0.25;
	float_move_animation.is_looping = true;
	anim_controller_.AddAnimation(U"float_move", float_move_animation);

	Animation swim_animation;
	swim_animation.texture_asset_names = { U"player_2", U"player_3" };
	swim_animation.frame_duration_sec = 0.07;
	swim_animation.is_looping = false;
	anim_controller_.AddAnimation(U"swim", swim_animation);
}

void Player::Update(const Stage& stage)
{
	HandleInput();
	UpdatePhysics(stage);
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
		is_facing_right_ = false;
	}
	else if(kInputRight.pressed())
	{
		velocity_.x = Min(velocity_.x + horizontal_accel_, horizontal_speed_max_);
		is_moving_x_ = true;
		is_facing_right_ = true;
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
	if(kInputAction1.down())
	{
		velocity_.y = swim_power_;

		// 泳ぎ始めのアニメーション再生のためにダミーアニメーションを再生
		anim_controller_.Play(U"float_idle");
	}
}

// 物理演算と位置更新
void Player::UpdatePhysics(const Stage& stage)
{
	// プレイヤーの当たり判定のサイズ(pos_が中央と仮定)
	constexpr double kPlayerHalfWidth = 25.0;
	constexpr double kPlayerHalfHeight = 62.0;
	const double tile_size = stage.GetTileSize();

	// Y軸の重力・速度計算
	if(velocity_.y < 0)
	{
		velocity_.y += (gravity_ * rising_gravity_multiplier_);
	}
	else
	{
		velocity_.y += gravity_;
	}
	velocity_.y = Min(velocity_.y, terminal_velocity_y_);

	// X軸の移動と衝突判定
	double next_x = pos_.x + velocity_.x;
	if(velocity_.x > 0)
	{
		// センサー: 右側面の上端と下端
		const double sensor_x = next_x + kPlayerHalfWidth;
		const double sensor_y_top = pos_.y - kPlayerHalfHeight + 1.0;
		const double sensor_y_bot = pos_.y + kPlayerHalfHeight - 1.0;
		if(stage.IsSolid(sensor_x, sensor_y_top) || stage.IsSolid(sensor_x, sensor_y_bot))
		{
			// 衝突，壁タイルの左端にスナップ
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - kPlayerHalfWidth;
			velocity_.x = 0; // 速度リセット
		}
		else
		{
			pos_.x = next_x;
		}
	}
	else if(velocity_.x < 0)
	{
		// センサー: 左側面の上端と下端
		const double sensor_x = next_x - kPlayerHalfWidth;
		const double sensor_y_top = pos_.y - kPlayerHalfHeight + 1.0;
		const double sensor_y_bot = pos_.y + kPlayerHalfHeight - 1.0;
		if(stage.IsSolid(sensor_x, sensor_y_top) || stage.IsSolid(sensor_x, sensor_y_bot))
		{
			// 衝突，壁タイルの右端にスナップ
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + kPlayerHalfWidth;
			velocity_.x = 0; // 速度リセット
		}
		else
		{
			pos_.x = next_x;
		}
	}

	// Y軸の移動と衝突判定
	double next_y = pos_.y + velocity_.y;
	is_grounded_ = false;

	if(velocity_.y > 0)
	{
		// センサー: 下面の左端と右端
		const double sensor_y = next_y + kPlayerHalfHeight;
		const double sensor_x_left = pos_.x - kPlayerHalfWidth + 1.0;
		const double sensor_x_right = pos_.x + kPlayerHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			// 衝突，床タイルの上端にスナップ
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) - kPlayerHalfHeight;
			velocity_.y = 0; // 速度リセット
			is_grounded_ = true; // 着地
		}
		else
		{
			pos_.y = next_y;
		}
	}
	else if(velocity_.y < 0)
	{
		// センサー: 上面の左端と右端
		const double sensor_y = next_y - kPlayerHalfHeight;
		const double sensor_x_left = pos_.x - kPlayerHalfWidth + 1.0;
		const double sensor_x_right = pos_.x + kPlayerHalfWidth - 1.0;
		if(stage.IsSolid(sensor_x_left, sensor_y) || stage.IsSolid(sensor_x_right, sensor_y))
		{
			// 衝突，天井タイルの下端にスナップ
			pos_.y = (std::floor(sensor_y / tile_size) * tile_size) + tile_size + kPlayerHalfHeight;
			velocity_.y = 0; // 速度リセット
		}
		else
		{
			pos_.y = next_y;
		}
	}

	// 動的Collider(vs 敵用)の位置を最終座標で更新
	collider.shape = RectF{ Arg::center(pos_), kPlayerHalfWidth * 2, kPlayerHalfHeight * 2 };
}

// アニメーション制御
void Player::UpdateAnimation()
{
	// swimアニメーションが再生中(または終了してスタック中)か確認
	if(anim_controller_.IsPlaying(U"swim"))
	{
		if(velocity_.y > 0) // 下に移動中なら
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"float_move");
			}
			else
			{
				anim_controller_.Play(U"float_idle");
			}
		}
	}
	else // swimが再生中でない場合
	{
		if(velocity_.y < 0) // 上に移動中なら
		{
			anim_controller_.Play(U"swim");
		}
		else if(is_grounded_) // 接地時
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"walk");
			}
			else
			{
				anim_controller_.Play(U"ground_idle");
			}
		}
		else // 空中(水中)時
		{
			if(is_moving_x_)
			{
				anim_controller_.Play(U"float_move");
			}
			else
			{
				anim_controller_.Play(U"float_idle");
			}
		}
	}
}

void Player::Draw(const Vec2& camera_offset) const
{
	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		// pos_(double)が中央座標だと仮定しているため，描画時に左上座標に補正
		const Vec2 top_left_pos = pos_ - Vec2{ 64, 64 };

		// スクリーン座標 = ワールド座標 - カメラオフセット(doubleのまま計算)
		const Vec2 draw_pos = top_left_pos - camera_offset;

		// 描画直前に s3d::Floor で整数にスナップ
		const Vec2 final_draw_pos = s3d::Floor(draw_pos);

		// 向きに応じて描画を分岐
		if(is_facing_right_)
		{
			texture_asset->mirrored().draw(final_draw_pos);
		}
		else
		{
			texture_asset->draw(final_draw_pos);
		}
	}
	else
	{
		// texture_assetの中身がなかった場合
		RectF{ Arg::center(s3d::Floor(pos_ - camera_offset)), 32, 32 }.drawFrame(2, 0, Palette::Red);
	}
}

Vec2 Player::GetPos() const { return pos_; }

void Player::SetPos(const Vec2& new_pos)
{
	pos_ = new_pos;
	velocity_ = Vec2::Zero();
}

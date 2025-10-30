#include "../World/Stage.h"
#include "Component/Animation.h"
#include "Enemy.h"
#include "Player.h"

#include <Siv3D.hpp>
#include <variant>

Enemy::Enemy(const String& type, const Vec2& center_pos)
	: pos_(center_pos)
{
	Animation anim;

	// Patrol型
	if(type == U"Fish")
	{
		behavior_ = EnemyBehavior::Patrol;
		size_ = { 32, 24 }; // 物理的な当たり判定のサイズ (幅, 高さ)
		velocity_.x = -kPatrolSpeed; // まず左に移動開始
		is_facing_right_ = false;

		anim.texture_asset_names = { U"fish1" };
		anim.frame_duration_sec = 0.25;
		anim.is_looping = true;
		anim_controller_.AddAnimation(U"move", anim);
		anim_controller_.Play(U"move");
	}
	// Stationary型
	else
	{
		behavior_ = EnemyBehavior::Stationary;
		velocity_ = Vec2::Zero();
		anim.is_looping = true;

		if(type == U"Coral_L")
		{
			size_ = { 64, 64 };
			anim.texture_asset_names = { U"coral_l" };
			anim.is_looping = false;
		}
		else if(type == U"Coral_R")
		{
			size_ = { 64, 64 };
			anim.texture_asset_names = { U"coral_r" };
			anim.is_looping = false;
		}
		else if(type == U"Clione")
		{
			size_ = { 18, 24 };
			anim.texture_asset_names = { U"clione1" };
			anim.frame_duration_sec = 0.5;
		}

		anim_controller_.AddAnimation(U"idle", anim);
		anim_controller_.Play(U"idle");
	}

	collider_ = Collider{
		RectF{ Arg::center(pos_), size_ },
		ColliderTag::kEnemy
	};
}

void Enemy::Update(const Stage& stage, const Player& player)
{
	if(not is_alive_) return;

	// 振る舞いに応じてロジックを更新
	if(behavior_ == EnemyBehavior::Patrol)
	{
		UpdatePatrol(stage);
	}
	// Stationaryの場合は何もしない

	anim_controller_.Update();

	// std::get<RectF>で形状を取り出し，位置を更新
	std::get<RectF>(collider_.shape).setCenter(pos_);

	// GameScene::update()で検知された結果を見る
	if(collider_.is_colliding)
	{
		for(const auto& tag : collider_.collided_tags)
		{
			if(tag == ColliderTag::kPlayer)
			{
			}
		}
	}
}

// 巡回ロジック
void Enemy::UpdatePatrol(const Stage& stage)
{
	// 重力は考慮しない

	const double next_x = pos_.x + velocity_.x;
	const double half_width = size_.x / 2.0;
	const double tile_size = stage.GetTileSize();

	if(velocity_.x > 0) // 右に移動中
	{
		// センサー: 右側面の中央
		const double sensor_x = next_x + half_width;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			// 衝突，壁タイルの左端にスナップ
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - half_width;
			velocity_.x *= -1.0; // 反転
			is_facing_right_ = false;
		}
		else
		{
			pos_.x = next_x; // 衝突なし
		}
	}
	else if(velocity_.x < 0) // 左に移動中
	{
		// センサー: 左側面の中央
		const double sensor_x = next_x - half_width;
		const double sensor_y = pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			// 衝突，壁タイルの右端にスナップ
			pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + half_width;
			velocity_.x *= -1.0; // 反転
			is_facing_right_ = true;
		}
		else
		{
			pos_.x = next_x; // 衝突なし
		}
	}
}

void Enemy::Draw(const Vec2& camera_offset) const
{
	if(not is_alive_) return;

	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		// pos_は中央座標なので，左上座標に補正
		//const Vec2 top_left_pos = pos_ - (size_ / 2.0);

		// スクリーン座標 = ワールド座標 - カメラオフセット
		//const Vec2 draw_pos = top_left_pos - camera_offset;

		// スクリーン座標 = ワールドの中心座標 - カメラオフセット
		const Vec2 draw_pos = pos_ - camera_offset;

		// 整数にスナップ
		const Vec2 final_draw_pos = s3d::Floor(draw_pos);

		// 向きに応じて描画を分岐
		if(is_facing_right_)
		{
			texture_asset->mirrored().drawAt(final_draw_pos);
		}
		else
		{
			texture_asset->drawAt(final_draw_pos);
		}
	}
}

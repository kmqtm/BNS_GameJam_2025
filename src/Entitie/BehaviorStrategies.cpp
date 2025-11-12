#include "../World/Stage.h"
#include "BehaviorStrategies.h"
#include "Enemy.h"
#include <cmath>

// Stationary: 何もしない
void StationaryBehavior::Update(Enemy& enemy, const Stage& stage)
{
	(void)enemy;
	(void)stage;
}

// Patrol: Enemy::UpdatePatrol のロジックを移植
void PatrolBehavior::Update(Enemy& enemy, const Stage& stage)
{
	const double next_x = enemy.pos_.x + enemy.velocity_.x;
	const double half_width = enemy.physics_size_.x / 2.0;
	const double tile_size = stage.GetTileSize();

	if(enemy.velocity_.x > 0) // 右に移動中
	{
		// collision_offset_を加えて早めに検知
		const double sensor_x = next_x + half_width + enemy.collision_offset_;
		const double sensor_y = enemy.pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			enemy.pos_.x = (std::floor(sensor_x / tile_size) * tile_size) - half_width - enemy.collision_offset_;
			enemy.velocity_.x *= -1.0;
			enemy.is_facing_right_ = false;
		}
		else
		{
			enemy.pos_.x = next_x;
		}
	}
	else if(enemy.velocity_.x < 0) // 左に移動中
	{
		// collision_offset_を加えて早めに検知
		const double sensor_x = next_x - half_width - enemy.collision_offset_;
		const double sensor_y = enemy.pos_.y;

		if(stage.IsSolid(sensor_x, sensor_y))
		{
			enemy.pos_.x = (std::floor(sensor_x / tile_size) * tile_size) + tile_size + half_width + enemy.collision_offset_;
			enemy.velocity_.x *= -1.0;
			enemy.is_facing_right_ = true;
		}
		else
		{
			enemy.pos_.x = next_x;
		}
	}
}

// BackAndForth: Enemy::UpdateBackAndForth のロジックを移植
void BackAndForthBehavior::Update(Enemy& enemy, const Stage& stage)
{
	(void)stage;

	// 現在の移動方向に従って位置を更新
	enemy.pos_.x += enemy.velocity_.x;

	// 開始位置からの移動距離を計算
	const double distance_from_start = std::abs(enemy.pos_.x - enemy.start_pos_.x);

	// 最大移動距離に達したら方向を反転
	if(distance_from_start >= enemy.max_travel_distance_)
	{
		// 方向反転（スプライトの向きは変えない）
		enemy.velocity_.x *= -1.0;

		// 距離をリセット
		enemy.travel_distance_ = 0.0;

		// 正確な位置に補正（行き過ぎを防ぐ）
		if(enemy.velocity_.x > 0)
		{
			enemy.pos_.x = enemy.start_pos_.x - enemy.max_travel_distance_;
		}
		else
		{
			enemy.pos_.x = enemy.start_pos_.x + enemy.max_travel_distance_;
		}
	}
}

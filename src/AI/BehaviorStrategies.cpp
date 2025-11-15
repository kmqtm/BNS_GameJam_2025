#include "../Entitie/Enemy.h"
#include "../World/Stage.h"
#include "BehaviorStrategies.h"
#include <cmath>

void StationaryBehavior::Update(Enemy& enemy, const Stage& stage)
{
	(void)enemy;
	(void)stage;
}

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
			// 右辺: タイル左端 - 敵幅/2 - オフセット
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
			// 右辺: タイル左端 + 敵幅/2 + オフセット
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

void BackAndForthBehavior::Update(Enemy& enemy, const Stage& stage)
{
	// ステージのパラメータは使用しない
	(void)stage;

	enemy.pos_.x += enemy.velocity_.x;

	const double distance_from_start = std::abs(enemy.pos_.x - enemy.start_pos_.x);

	// 最大移動距離に達したら方向を反転(スプライトの向きは変えない)
	if(distance_from_start >= enemy.max_travel_distance_)
	{
		enemy.velocity_.x *= -1.0;
		enemy.travel_distance_ = 0.0;

		// 正確な位置に補正
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

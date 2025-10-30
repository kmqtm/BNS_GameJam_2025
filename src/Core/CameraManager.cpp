#include "../Core/Utility.h"
#include "CameraManager.h"

#include <Siv3D.hpp>

CameraManager::CameraManager(double fixed_world_x, const Size& view_size)
	: fixed_world_x_(fixed_world_x)
	, y_offset_(view_size.y* (1.0 / 6.0))
	, view_size_(view_size)
	, target_y_(0.0)
	, current_y_(target_y_ + y_offset_)
{
	camera_.setCenter(Vec2{ fixed_world_x_, current_y_ });
}

void CameraManager::SetTargetY(double target_y)
{
	target_y_ = target_y;
}

void CameraManager::Update()
{
	// カメラの中心が向かうべき目標Y座標を計算
	const double goal_y = target_y_ + y_offset_;

	// 現在のカメラY座標を，目標Y座標に滑らかに近づける(1.0 にすると即座に追従)
	current_y_ = Math::Lerp(current_y_, goal_y, 0.1);

	// カメラの中心座標を(固定X, 計算したY)に設定
	camera_.setCenter(Vec2{ fixed_world_x_, current_y_ });
	camera_.update();
}

Vec2 CameraManager::GetCameraOffset() const
{
	// 中心座標を取得
	const Vec2 center = Vec2{ fixed_world_x_, current_y_ };

	// ビューサイズの半分を計算
	const Vec2 half_view_size = (view_size_ / 2.0);

	return (center - half_view_size);
}

RectF CameraManager::GetViewRect() const
{
	return RectF{ GetCameraOffset(), view_size_ };
}

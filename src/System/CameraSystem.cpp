#include "../Core/Utility.h"
#include "CameraSystem.h"

#include <Siv3D.hpp>

CameraSystem::CameraSystem(double fixed_world_x, const Size& view_size)
	: fixed_world_x_(fixed_world_x)
	, y_offset_(view_size.y* (1.0 / 6.0))
	, view_size_(view_size)
	, target_y_(300.0)
	, current_y_(target_y_ + y_offset_)
{
	camera_.setCenter(Vec2{ fixed_world_x_, current_y_ });
}

void CameraSystem::SetTargetY(double target_y)
{
	target_y_ = target_y;
}

void CameraSystem::SetYOffsetRatio(double ratio)
{
	y_offset_ = view_size_.y * ratio;
}

// カメラ中心の目標Yを計算
double CameraSystem::ComputeGoalY() const
{
	return target_y_ + y_offset_;
}

// 補間ヘルパー（線形補間）
double CameraSystem::SmoothTo(double current, double goal, double factor) const
{
	return Math::Lerp(current, goal, factor);
}

// カメラ中心座標を計算
Vec2 CameraSystem::ComputeCameraCenter() const
{
	return Vec2{ fixed_world_x_, current_y_ };
}

Vec2 CameraSystem::HalfViewSize() const
{
	return (view_size_ / 2.0);
}

void CameraSystem::Update()
{
	// 目標Yを計算し、現在値を滑らかに更新
	const double goal_y = ComputeGoalY();
	current_y_ = SmoothTo(current_y_, goal_y, 0.05);

	// カメラに適用
	camera_.setCenter(ComputeCameraCenter());
	camera_.update();
}

Vec2 CameraSystem::GetCameraOffset() const
{
	// 中心 - ビュー半分 = 左上のワールド座標
	const Vec2 center = ComputeCameraCenter();
	return (center - HalfViewSize());
}

RectF CameraSystem::GetViewRect() const
{
	return RectF{ GetCameraOffset(), view_size_ };
}

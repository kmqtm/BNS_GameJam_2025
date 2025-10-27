#include "CameraManager.h"

namespace Core
{
	CameraManager::CameraManager()
		: camera_{ s3d::Vec2{ 0, 0 }, 1.0 ,s3d::CameraControl::None_ }
	{
	}

	void CameraManager::Update()
	{
		camera_.update();
	}

	void CameraManager::SetTarget(const s3d::Vec2& target)
	{
		camera_.setTargetCenter(target);
	}

	s3d::Vec2 CameraManager::GetSnappedOffset() const
	{
		const s3d::Vec2 camera_center = camera_.getCenter();
		const s3d::Vec2 camera_top_left = camera_center - s3d::Scene::Center();

		return Utility::RoundVec2(camera_top_left);
	}

	s3d::RectF CameraManager::GetViewRect() const
	{
		return camera_.getRegion();
	}
}

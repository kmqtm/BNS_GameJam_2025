#include "CameraManager.h"

namespace Core
{
	CameraManager::CameraManager()
		// GameScene での初期化設定をこちらに移動
		: camera_{ s3d::Vec2{ 0, 0 }, 1.0 ,s3d::CameraControl::None_ }
	{
		// コンストラクタ
	}

	void CameraManager::Update()
	{
		// GameScene::update() で呼んでいたカメラ自体の更新
		camera_.update();
	}

	void CameraManager::SetTarget(const s3d::Vec2& target)
	{
		// GameScene::update() で呼んでいたターゲット設定
		camera_.setTargetCenter(target);
	}

	s3d::Vec2 CameraManager::GetSnappedOffset() const
	{
		// GameScene::draw() で行っていたスナップ処理
		const s3d::Vec2 camera_center = camera_.getCenter();
		const s3d::Vec2 camera_top_left = camera_center - s3d::Scene::Center();

		// Utility::RoundVec2 を使用
		return Utility::RoundVec2(camera_top_left);
	}

	s3d::RectF CameraManager::GetViewRect() const
	{
		// GameScene::draw() で行っていたビュー矩形の取得
		return camera_.getRegion();
	}
}
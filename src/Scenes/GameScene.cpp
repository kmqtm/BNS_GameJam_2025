#include "../Core/AssetController.h"
#include "../Core/Config.h"
#include "../Core/Utility.h" // Utility::RoundVec2 を使うため
#include "GameScene.h"

#include <Siv3D.hpp>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init)
{
	AssetController::GetInstance().PrepareAssets(U"Game");
}

GameScene::~GameScene()
{
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::update()
{
	// 1. プレイヤーの状態を更新
	player_.Update();

	// 2. カメラの「目標」位置をプレイヤーの現在位置に設定
	Vec2 target = player_.GetPos();
	camera_.setTargetCenter(target);

	// 3. カメラの現在位置を，設定された目標位置 (target) に向かって移動
	camera_.update();
}


void GameScene::draw() const
{
	Scene::SetBackground(kGameBackgroundColor);

	const Vec2 camera_center = camera_.getCenter();
	const Vec2 camera_top_left = camera_center - Scene::Center();
	const Vec2 snapped_camera_offset = Utility::RoundVec2(camera_top_left);

	const RectF view_rect = camera_.getRegion();

	stage_.Draw(snapped_camera_offset, view_rect);

	player_.Draw(snapped_camera_offset);

	// UI
}
#include "../Core/AssetController.h"
#include "../Core/Config.h"
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
	player_.Update(stage_);

	Vec2 target = player_.GetPos();
	camera_manager_.SetTarget(target);

	camera_manager_.Update();
}


void GameScene::draw() const
{
	Scene::SetBackground(kGameBackgroundColor);

	const Vec2 snapped_camera_offset = camera_manager_.GetSnappedOffset();
	const RectF view_rect = camera_manager_.GetViewRect();

	stage_.Draw(snapped_camera_offset, view_rect);

	player_.Draw(snapped_camera_offset);

	// UI
}

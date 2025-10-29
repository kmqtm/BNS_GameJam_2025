#include "../Core/AssetController.h"
#include "../Core/Config.h"
#include "GameScene.h"

#include <Siv3D.hpp>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init)
	, camera_manager_(
		(stage_.GetWidth()* stage_.GetTileSize()) / 2.0,		// 固定するX座標 (ステージ中央)
		kSceneSize											// 画面サイズ (Yオフセット計算用)
	)
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

	camera_manager_.SetTargetY(player_.GetPos().y);

	camera_manager_.Update();
}


void GameScene::draw() const
{
	Scene::SetBackground(kGameBackgroundColor);

	// スナップされていない(doubleの)オフセットを取得
	const Vec2 camera_offset = camera_manager_.GetCameraOffset();
	const RectF view_rect = camera_manager_.GetViewRect();

	stage_.Draw(camera_offset, view_rect);

	player_.Draw(camera_offset);

	// UI
}

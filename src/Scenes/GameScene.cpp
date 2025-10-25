#include "GameScene.h"
#include "../Core/AssetController.h" // AssetController (Singleton)
#include "../Core/Config.h"         // kGameBackgroundColor

#include <Siv3D.hpp>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init) {
	AssetController::GetInstance().PrepareAssets(U"Game");

	// カメラ初期位置（プレイヤーの位置などに合わせる予定）
	m_camera.setCenter(Vec2{ 0, 0 });
}

GameScene::~GameScene() {
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::update()
{
	Vec2 target = m_camera.getTargetCenter();

	const double moveSpeed = 8.0; // スクロール速度（ピクセル／フレーム）

	if (KeyRight.pressed()) target.x += moveSpeed;
	if (KeyLeft.pressed())  target.x -= moveSpeed;
	if (KeyUp.pressed())    target.y -= moveSpeed;
	if (KeyDown.pressed())  target.y += moveSpeed;

	// 目標位置を直接更新
	m_camera.setTargetCenter(target);

	// カメラ更新（補間あり）
	m_camera.update();
}


void GameScene::draw() const {
	Scene::SetBackground(kGameBackgroundColor);

	// カメラ中心を取得（そのままワールド座標）
	const Vec2 cameraCenter = m_camera.getCenter();

	// ステージをカメラのオフセットで描画（ピクセルスナップ済み）
	m_stage.draw(cameraCenter);

	// UIなどはカメラ外に描く
	// FontAsset(U"UI")(U"Game Scene").draw(10, 10);
}

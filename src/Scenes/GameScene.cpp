#include "GameScene.h"

#include "../Core/AssetController.h"
#include "../Core/Config.h"

#include <Siv3D.hpp>
#include <variant>

GameScene::GameScene(const App::Scene::InitData &init)
	: IScene(init) {
	AssetController::GetInstance().PrepareAssets(U"Game");

	// 背景を水色に設定
	Scene::SetBackground(kGameBackgroundColor);
}

GameScene::~GameScene() {
	// 現在のシーン内で使用されているアセットを登録解除
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::update() {
	//player_.Update();
}

void GameScene::draw() const {
}



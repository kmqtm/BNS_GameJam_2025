#include "GameScene.h"
#include "../Core/AssetController.h" // AssetController (Singleton)
#include "../Core/Config.h"         // kGameBackgroundColor

#include <Siv3D.hpp>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init) {
	// シーンで使用するアセット（プレイヤー, UI, SEなど）をロード
	// ステージタイルセットは Stage クラス自身がロードするため，ここでは不要
	AssetController::GetInstance().PrepareAssets(U"Game");
}

GameScene::~GameScene() {
	// このシーンでロードしたアセットをまとめて登録解除
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::update()
{
	// 現在のカメラの「目標」位置を取得
	Vec2 target = m_camera.getTargetCenter();

	// フレームレートに依存しないよう，Scene::DeltaTime() を使うのが望ましいが
	// ここでは固定値（ピクセル／フレーム）でスクロール速度を定義
	const double moveSpeed = 8.0;

	if (KeyRight.pressed()) target.x += moveSpeed;
	if (KeyLeft.pressed())  target.x -= moveSpeed;
	if (KeyUp.pressed())    target.y -= moveSpeed;
	if (KeyDown.pressed())  target.y += moveSpeed;

	// ユーザー入力に基づき，カメラの「目標」位置を更新
	m_camera.setTargetCenter(target);

	// カメラの現在位置を，設定された目標位置 (target) に向かって
	// スムーズに補間（Lerp）移動させる
	m_camera.update();
}


void GameScene::draw() const {
	Scene::SetBackground(kGameBackgroundColor);

	// カメラの現在（補間済み）のワールド座標を取得
	const Vec2 cameraCenter = m_camera.getCenter();

	// ステージを描画。カメラのワールド座標をオフセットとして渡す
	// これにより，Stage側で (ワールド座標 - カメラ座標) の計算が行われる
	m_stage.draw(cameraCenter);

	// UIなどはカメラの影響を受けないスクリーン座標で描画する
	// (現在はコメントアウト)
	// FontAsset(U"UI")(U"Game Scene").draw(10, 10);
}

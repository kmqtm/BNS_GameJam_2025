#include "../Core/AssetController.h"
#include "../Core/Config.h"
#include "GameScene.h"

#include <Siv3D.hpp>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init)
{
	// シーンで使用するアセット（プレイヤー, UI, SEなど）をロード
	// ステージタイルセットは Stage クラス自身がロードするため，ここでは不要
	AssetController::GetInstance().PrepareAssets(U"Game");
}

GameScene::~GameScene()
{
	// このシーンでロードしたアセットをまとめて登録解除
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::update()
{
	// 1. プレイヤーの状態を更新 (入力処理と物理演算はこちらで行われる)
	player_.Update();

	// 2. カメラの「目標」位置をプレイヤーの現在位置に設定
	// (元の矢印キーによるカメラ操作は削除)
	Vec2 target = player_.GetPos();
	camera_.setTargetCenter(target);

	// 3. カメラの現在位置を，設定された目標位置 (target) に向かって
	//    スムーズに補間（Lerp）移動させる
	camera_.update();
}


void GameScene::draw() const
{
	Scene::SetBackground(kGameBackgroundColor);

	// カメラの現在（補間済み）のワールド座標を取得
	const Vec2 cameraCenter = camera_.getCenter();

	// ステージを描画。カメラのワールド座標をオフセットとして渡す
	// これにより，Stage側で (ワールド座標 - カメラ座標) の計算が行われる
	stage_.Draw(cameraCenter);

	{
		// 2Dカメラの描画範囲に切り替え
		const auto t = camera_.createTransformer();

		// このブロック内で描画されたものはワールド座標として扱われる
		player_.Draw();
	}

	// UIなどはカメラの影響を受けないスクリーン座標で描画する
	// (現在はコメントアウト)
	// FontAsset(U"UI")(U"Game Scene").draw(10, 10);
}

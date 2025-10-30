#include "../Core/AssetController.h"
#include "../Core/Config.h"
#include "../World/SpawnInfo.h"
#include "GameScene.h"

#include <Siv3D.hpp>
#include <variant>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init)
	, camera_manager_(
		(stage_.GetWidth()* stage_.GetTileSize()) / 2.0,		// 固定するX座標 (ステージ中央)
		kSceneSize											// 画面サイズ (Yオフセット計算用)
	)
{
	AssetController::GetInstance().PrepareAssets(U"Game");

	SpawnEnemies();
}

GameScene::~GameScene()
{
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::SpawnEnemies()
{
	// Stageからスポーン情報を取得
	const auto& spawn_points = stage_.GetSpawnPoints();

	// 情報を元に敵を生成
	for(const auto& info : spawn_points)
	{
		if(info.type.isEmpty())
		{
			Print << U"Warning: Tiled object_spawn に 'Type' が設定されていないオブジェクトがあります．";
			continue;
		}

		// 座標変換
		// Tiledの座標(info.pos)は左上
		// Enemyコンストラクタ (Enemy.cpp) は「中心座標」を期待している
		// 
		// info.pos (左上) + info.size / 2.0 (サイズの半分) = 中心座標
		const Vec2 center_pos = info.pos + (info.size / 2.0);

		// 4. Enemyを生成
		enemies_.emplace_back(info.type, center_pos);
	}

	Print << U"Spawned {} enemies."_fmt(enemies_.size());
}

void GameScene::update()
{
	player_.Update(stage_);
	for(auto& enemy : enemies_)
	{
		enemy.Update(stage_, player_);
	}

	camera_manager_.SetTargetY(player_.GetPos().y);
	camera_manager_.Update();

	// 全Colliderの衝突状態をリセット
	player_.collider.ClearCollisionResult();
	for(auto& enemy : enemies_)
	{
		enemy.GetCollider().ClearCollisionResult();
	}

	auto& player_collider = player_.collider;

	for(auto& enemy : enemies_)
	{
		if(not enemy.IsAlive()) continue;

		auto& enemy_collider = enemy.GetCollider();

		bool is_collided = std::visit([&](const auto& player_shape)
									  {
										  return std::visit([&](const auto& enemy_shape)
															{
																return player_shape.intersects(enemy_shape);
															}, enemy_collider.shape);
									  }, player_collider.shape);

		if(is_collided)
		{
			player_collider.is_colliding = true;
			player_collider.collided_tags.push_back(enemy_collider.tag);

			enemy_collider.is_colliding = true;
			enemy_collider.collided_tags.push_back(player_collider.tag);
		}
	}
}


void GameScene::draw() const
{
	Scene::SetBackground(kGameBackgroundColor);

	// スナップされていない(doubleの)オフセットを取得
	const Vec2 camera_offset = camera_manager_.GetCameraOffset();
	const RectF view_rect = camera_manager_.GetViewRect();

	player_.Draw(camera_offset);

	stage_.Draw(camera_offset, view_rect);

	for(const auto& enemy : enemies_)
	{
		enemy.Draw(camera_offset);
	}

	// UI
}

#pragma once

#include "../Core/CameraManager.h"
#include "../Core/Config.h"
#include "../Entitie/Enemy.h"
#include "../Entitie/Player.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

class GameScene : public App::Scene
{
public:
	GameScene(const InitData& init);
	~GameScene() override;

	void update() override;
	void draw() const override;

private:
	// Tiledからスポーン情報を読み込み，enemies_配列を初期化
	void SpawnEnemies();

	// stage_を先に宣言(CameraManagerの初期化で使うため)
	Stage stage_{ U"asset/Stage/v2/tilemap.json", U"asset/Stage/v2/tileset.png", U"collision_layer" };

	CameraManager camera_manager_;

	Player player_;

	s3d::Array<Enemy> enemies_;
};

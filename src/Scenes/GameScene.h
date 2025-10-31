#pragma once

#include "../Core/CameraManager.h"
#include "../Core/Config.h"
#include "../Entitie/Enemy.h"
#include "../Entitie/OxygenSpot.h"
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
	void SpawnEntities();

	void OnPlayerDied();

	// stage_を先に宣言(CameraManagerの初期化で使うため)
	Stage stage_{ U"asset/Stage/v3/tilemap.json", U"asset/Stage/v3/tileset.png", U"collision_layer" };

	CameraManager camera_manager_;
	Player player_;
	bool is_player_dead_ = false;
	s3d::Array<Enemy> enemies_;
	s3d::Array<OxygenSpot> oxygen_spots_;
};

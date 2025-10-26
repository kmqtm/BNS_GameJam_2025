#pragma once

#include "../Core/CameraManager.h"
#include "../Core/Config.h"
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
	Stage stage_{ U"asset/Stage/tilemap_v0.json", U"asset/Stage/stage_tiles_v0.png" };

	Core::CameraManager camera_manager_;

	Player player_;
};
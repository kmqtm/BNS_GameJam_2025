#pragma once

#include "../Core/Config.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

// メインのゲームプレイシーン
class GameScene : public App::Scene {
public:
	GameScene(const InitData& init);
	~GameScene() override;

	void update() override;
	void draw() const override;

private:
	Stage m_stage{ U"asset/Stage/tilemap_v0.json", U"asset/Stage/stage_tiles_v0.png" };
	Camera2D m_camera{ Vec2{ 0, 0 }, 1.0 ,CameraControl::None_ };
};

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

	void DrawOxygenGauge() const;
	void DrawProgressMeter() const;

	// stage_を先に宣言(CameraManagerの初期化で使うため)
	Stage stage_{ U"asset/Stage/v3/tilemap.json", U"asset/Stage/v3/tileset.png", U"collision_layer" };

	CameraManager camera_manager_;
	Player player_;
	bool is_player_dead_ = false;
	s3d::Array<Enemy> enemies_;
	s3d::Array<OxygenSpot> oxygen_spots_;

	// 進行度メーター用の変数
	double player_start_y_ = 0.0;
	double map_total_height_ = 0.0;

	// UIの定数
	static constexpr Vec2 kOxygenGaugePos = { 20, 20 };
	static constexpr Size kOxygenGaugeSize = { 24, 200 };
	static constexpr ColorF kOxygenGaugeColor = Palette::Cyan;
	static constexpr ColorF kUIGaugeBackgroundColor = ColorF{ 0.0, 0.5 }; // 半透明の黒

	static constexpr double kProgressMeterWidth = 10.0;
	static constexpr ColorF kProgressMeterColor = Palette::Yellow;
};

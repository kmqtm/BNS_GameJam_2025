#pragma once

#include "../Core/CameraManager.h"
#include "../Core/Config.h"
#include "../Entitie/Component/SoundController.h"
#include "../Entitie/Enemy.h"
#include "../Entitie/OxygenSpot.h"
#include "../Entitie/Player.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

enum class GameState
{
	Title,
	Playing,
	Ending,
	GameOver
};

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

	Vec2 FindNearestRespawnSpot() const;

	void UpdateBGM();

	// stage_を先に宣言(CameraManagerの初期化で使うため)
	Stage stage_{ U"asset/Stage/v3/tilemap_v3.json", U"asset/Stage/v3/tileset.png", U"collision_layer" };

	CameraManager camera_manager_;
	Player player_;
	GameState current_state_ = GameState::Title;
	s3d::Array<Enemy> enemies_;
	s3d::Array<OxygenSpot> oxygen_spots_;

	SoundController bgm_controller_;
	bool is_intro_finished_ = false;

	Vec2 player_start_pos_ = Vec2::Zero();
	double map_total_height_ = 0.0;

	static constexpr Vec2 kOxygenGaugePos = { 20, 20 };
	static constexpr Size kOxygenGaugeSize = { 24, 200 };
	static constexpr ColorF kUIGaugeBackgroundColor = ColorF{ 0.0, 0.5 };

	static constexpr ColorF kOxygenColorSafe = Palette::Limegreen;
	static constexpr ColorF kOxygenColorWarning = Palette::Yellow;
	static constexpr ColorF kOxygenColorDanger = Palette::Red;

	static constexpr double kOxygenWarningThreshold = 70.0;
	static constexpr double kOxygenDangerThreshold = 30.0;

	// 深度による背景色
	static constexpr ColorF kDeepSeaColor = ColorF{ 0.0, 0.1, 0.3 }; // 紺色

	static constexpr double kProgressMeterWidth = 10.0;

	static constexpr ColorF kProgressLineColor = Palette::White;			// メーターの縦線の色
	static constexpr ColorF kProgressPlayerColor = Palette::Yellow;		// プレイヤーマーカーの色
	static constexpr ColorF kProgressSpotColor = Palette::Limegreen;	// 酸素スポットマーカーの色
	static constexpr double kProgressPlayerMarkerWidth = 14.0;			// プレイヤーマーカーの幅
	static constexpr double kProgressSpotMarkerWidth = 8.0;				// スポットマーカーの幅
	static constexpr double kProgressMarkerHeight = 4.0;				// マーカーの高さ

	// タイトル・エンディング画面用のカメラオフセット
	static constexpr double kTitleEndingCameraOffsetYRatio = -1.0 / 4.9;
	// ゲームプレイ用のカメラオフセット(上1/3にPlayer)
	static constexpr double kPlayingCameraOffsetYRatio = 1.0 / 6.0;

	// エンディングに移行するY座標
	static constexpr double kEndingZoneY = 7650.0;
};

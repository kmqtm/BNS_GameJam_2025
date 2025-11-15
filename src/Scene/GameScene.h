#pragma once

/// @file GameScene.h
/// @brief ゲームシーンの定義

#include "../Component/SoundController.h"
#include "../Core/Config.h"
#include "../Entitie/Enemy.h"
#include "../Entitie/OxygenSpot.h"
#include "../Entitie/Player.h"
#include "../System/CameraSystem.h"
#include "../System/CollisionSystem.h"
#include "../System/UISystem.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

/// @brief ゲームの状態
enum class GameState
{
	Title,      ///< タイトル画面
	Playing,    ///< ゲーム中
	Ending,     ///< エンディング
	GameOver    ///< ゲームオーバー
};

/// @brief ゲームシーンを管理する
class GameScene : public App::Scene
{
public:
	/// @brief コンストラクタ
	/// @param init 初期化データ
	GameScene(const InitData& init);

	/// @brief デストラクタ
	~GameScene() override;

	/// @brief シーンを更新する
	void update() override;

	/// @brief シーンを描画する
	void draw() const override;

private:
	/// @brief エンティティをスポーンする
	void SpawnEntities();

	/// @brief プレイヤーが死亡した時の処理
	void OnPlayerDied();

	/// @brief 最も近いリスポーン地点を探索する
	/// @return リスポーン地点の座標
	Vec2 FindNearestRespawnSpot() const;

	/// @brief BGMを更新する
	void UpdateBGM();

	/// @brief BGM再生を開始または遅延させる
	/// @param asset BGMのアセット名
	/// @param loop ループ再生するか
	void StartOrDeferBGM(const String& asset, bool loop);

	/// @brief 保留中のBGM再生を処理する
	void ProcessPendingBGM();

	/// @brief ステージ
	Stage stage_{ U"asset/Stage/v3/tilemap_v3.json", U"asset/Stage/v3/tileset.png", U"collision_layer" };

	/// @brief カメラシステム
	CameraSystem camera_manager_;

	/// @brief プレイヤー
	Player player_;

	/// @brief 現在のゲーム状態
	GameState current_state_ = GameState::Title;

	/// @brief 敵のリスト
	s3d::Array<Enemy> enemies_;

	/// @brief 酸素スポットのリスト
	s3d::Array<OxygenSpot> oxygen_spots_;

	/// @brief 衝突判定システム
	CollisionSystem collision_manager_;

	/// @brief BGM制御
	SoundController bgm_controller_;

	/// @brief イントロが終了したか
	bool is_intro_finished_ = false;

	/// @brief 保留中のBGMアセット名
	String pending_bgm_asset_;

	/// @brief 保留中のBGMループフラグ
	bool pending_bgm_loop_ = false;

	/// @brief 現在再生中のBGMアセット名
	String current_playing_bgm_asset_;

	/// @brief プレイヤーの開始位置
	Vec2 player_start_pos_ = Vec2::Zero();

	/// @brief ステージの総高さ
	double map_total_height_ = 0.0;

	/// @brief 背景オブジェクトのアクティブ化時刻
	mutable std::unordered_map<String, double> background_activation_times_;

	/// @brief 深海の背景色
	static constexpr ColorF kDeepSeaColor = ColorF{ 0.0, 0.1, 0.3 };

	/// @brief タイトル・エンディング画面用のカメラオフセット比率
	static constexpr double kTitleEndingCameraOffsetYRatio = -1.0 / 4.9;

	/// @brief ゲームプレイ用のカメラオフセット比率
	static constexpr double kPlayingCameraOffsetYRatio = 1.0 / 6.0;

	/// @brief エンディングに移行するY座標
	static constexpr double kEndingZoneY = 7650.0;

	/// @brief エンディング開始からの経過時間
	double ending_start_time_ = -1.0;

	/// @brief エンディング開始から笑顔に切り替わるまでの時間(秒)
	static constexpr double kOctopusSmileDelay = 7.0 + 8.6;

	/// @brief 笑顔後の画面暗転アルファ値
	static constexpr double kEndingDarkenAlpha = 0.45;

	/// @brief エンディング画像のアセット名
	static constexpr StringView kEndingOverlayTexture = U"ending_text";

	/// @brief UIシステム
	UISystem ui_manager_;
};

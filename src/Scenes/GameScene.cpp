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
	, map_total_height_(stage_.GetHeight()* stage_.GetTileSize())
{
	AssetController::GetInstance().PrepareAssets(U"Game");

	SpawnEntities();

	camera_manager_.SetTargetY(player_.GetPos().y);
	camera_manager_.SetYOffsetRatio(kTitleEndingCameraOffsetYRatio);
}

GameScene::~GameScene()
{
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::SpawnEntities()
{
	// Stageからスポーン情報を取得
	const auto& spawn_points = stage_.GetSpawnPoints();

	for(const auto& info : spawn_points)
	{
		if(info.type.isEmpty())
		{
			Print << U"Warning: Tiled object_spawn に 'Type' が設定されていないオブジェクトがあります．";
			continue;
		}

		const Vec2 center_pos = info.pos + (info.size / 2.0);

		if(info.type == U"Player")
		{
			player_.SetPos(center_pos);	// プレイヤーの初期位置
			player_start_pos_ = center_pos;
		}
		else if(info.type == U"Oxygen")
		{
			oxygen_spots_.emplace_back(center_pos, info.size);	// 酸素回復スポット
		}
		else // その他の敵type("Fish", "Coral_L"など)
		{
			enemies_.emplace_back(info.type, center_pos);
		}
	}
}

void GameScene::update()
{
	switch(current_state_)
	{
	case GameState::Title:
	{
		// OxygenSpotのアニメーションだけ更新
		for(auto& spot : oxygen_spots_)
		{
			spot.Update();
		}

		// ゲーム開始
		if(kInputOK.down() || KeyEnter.down())
		{
			current_state_ = GameState::Playing;
		}

		camera_manager_.SetYOffsetRatio(kTitleEndingCameraOffsetYRatio);
		break;
	}
	case GameState::Playing:
	{
		player_.Update(stage_);

		// エンディング判定
		if(player_.GetPos().y >= kEndingZoneY)
		{
			current_state_ = GameState::Ending;

			// カメラのワールド中心 X を取得してプレイヤーに渡す（プレイヤー側で徐々に中央へ移動させる）
			const double camera_center_x = camera_manager_.GetViewRect().center().x;
			player_.StartEnding(camera_center_x);
			break;
		}

		// 死亡判定
		if(player_.IsOxygenEmpty())
		{
			current_state_ = GameState::GameOver;
			OnPlayerDied();
			break;
		}

		for(auto& spot : oxygen_spots_)
		{
			spot.Update();
		}
		for(auto& enemy : enemies_)
		{
			enemy.Update(stage_, player_);
		}

		player_.collider.ClearCollisionResult();
		for(auto& enemy : enemies_) { enemy.GetCollider().ClearCollisionResult(); }
		for(auto& spot : oxygen_spots_) { spot.GetCollider().ClearCollisionResult(); }
		auto& player_collider = player_.collider;
		for(auto& enemy : enemies_)
		{
			if(not enemy.IsAlive()) continue;
			auto& enemy_collider = enemy.GetCollider();
			bool is_collided = std::visit([&](const auto& s1) { return std::visit([&](const auto& s2) { return s1.intersects(s2); }, enemy_collider.shape); }, player_collider.shape);
			if(is_collided)
			{
				player_collider.is_colliding = true;
				player_collider.collided_tags.push_back(enemy_collider.tag);

				enemy_collider.is_colliding = true;
				enemy_collider.collided_tags.push_back(player_collider.tag);
			}
		}
		for(auto& spot : oxygen_spots_)
		{
			auto& spot_collider = spot.GetCollider();
			bool is_collided = std::visit([&](const auto& s1) { return std::visit([&](const auto& s2) { return s1.intersects(s2); }, spot_collider.shape); }, player_collider.shape);
			if(is_collided)
			{
				player_collider.is_colliding = true;
				player_collider.collided_tags.push_back(spot_collider.tag);
				player_.RecoverOxygen();
			}
		}

		camera_manager_.SetYOffsetRatio(kPlayingCameraOffsetYRatio);
		break;
	}
	case GameState::Ending:
	{
		// フェード処理は削除：プレイヤーの移動は Player::Update()（内部Lerp）に任せる

		// プレイヤーはアニメーションと物理演算だけ更新(入力や酸素消費はしない)
		player_.Update(stage_);
		// OxygenSpot のアニメーションだけ更新（敵は更新しない）
		for(auto& spot : oxygen_spots_) { spot.Update(); }

		camera_manager_.SetYOffsetRatio(kTitleEndingCameraOffsetYRatio);
		break;
	}
	case GameState::GameOver:
	{
		player_.Update(stage_);

		if(kInputOK.down() || KeyEnter.down())
		{
			Vec2 respawn_pos = FindNearestRespawnSpot();
			player_.Respawn(respawn_pos);
			current_state_ = GameState::Playing;
		}

		camera_manager_.SetYOffsetRatio(kPlayingCameraOffsetYRatio);
		break;
	}
	}

	camera_manager_.SetTargetY(player_.GetPos().y);
	camera_manager_.Update();
}

void GameScene::OnPlayerDied()
{
	// ゲームオーバー演出

	Print << U"GAME SCENE: OXYGEN ZERO!";
}

Vec2 GameScene::FindNearestRespawnSpot() const
{
	const double dead_y = player_.GetPos().y;
	Vec2 best_spot_pos = Vec2::Zero();
	double min_distance = std::numeric_limits<double>::max();

	// プレイヤーより上にある酸素スポットを全探索
	for(const auto& spot : oxygen_spots_)
	{
		const Vec2 spot_pos = spot.GetPos();

		// スポットがプレイヤーより上にあるか
		if(spot_pos.y < dead_y)
		{
			const double distance_y = dead_y - spot_pos.y; // 垂直距離
			if(distance_y < min_distance)
			{
				min_distance = distance_y;
				best_spot_pos = spot_pos;
			}
		}
	}

	// スポットが1つも見つからなかったら
	if(best_spot_pos == Vec2::Zero())
	{
		return player_start_pos_; // 最初のスタート地点に戻す
	}

	return best_spot_pos;
}

void GameScene::draw() const
{
	static constexpr ColorF kSurfaceColor = kGameBackgroundColor;

	// 深度の割合(0.0 ~ 1.0)を計算
	const double total_travel = map_total_height_ - player_start_pos_.y;
	double depth_ratio = 0.0;
	if(total_travel > 0)
	{
		depth_ratio = (player_.GetPos().y - player_start_pos_.y) / total_travel;
	}
	depth_ratio = Clamp(depth_ratio, 0.0, 1.0);

	const ColorF current_bg_color = kSurfaceColor.lerp(kDeepSeaColor, depth_ratio);
	Scene::SetBackground(current_bg_color);

	// スナップされていない(doubleの)オフセットを取得
	const Vec2 camera_offset = camera_manager_.GetCameraOffset();
	const RectF view_rect = camera_manager_.GetViewRect();

	player_.Draw(camera_offset);

	stage_.Draw(camera_offset, view_rect);

	for(const auto& enemy : enemies_)
	{
		enemy.Draw(camera_offset);
	}

	for(const auto& spot : oxygen_spots_)
	{
		spot.Draw(camera_offset);
	}

	// UI
	DrawOxygenGauge();
	DrawProgressMeter();

	if(current_state_ == GameState::Title)
	{
	}
	else if(current_state_ == GameState::Ending)
	{
	}
	else if(current_state_ == GameState::GameOver)
	{
	}

	// 暗転・明転の描画は削除しました（フェード効果なし）
}

void GameScene::DrawOxygenGauge() const
{
	RectF{ kOxygenGaugePos, kOxygenGaugeSize }.draw(kUIGaugeBackgroundColor);

	const double current_oxygen = player_.GetOxygen();
	const double max_oxygen = player_.GetMaxOxygen();

	const double oxygen_ratio = (current_oxygen / max_oxygen);

	// 酸素量に応じて色を決定
	ColorF gauge_color;
	if(current_oxygen > kOxygenWarningThreshold) // 70% ~
	{
		gauge_color = kOxygenColorSafe;
	}
	else if(current_oxygen > kOxygenDangerThreshold) // 30% ~ 70%
	{
		gauge_color = kOxygenColorWarning;
	}
	else // ~ 30%
	{
		gauge_color = kOxygenColorDanger;
	}

	// ゲージの現在の高さ
	const double current_height = kOxygenGaugeSize.y * oxygen_ratio;

	// ゲージの描画(Y座標を調整して下から伸びるように)
	RectF{
		kOxygenGaugePos.x,
		kOxygenGaugePos.y + (kOxygenGaugeSize.y - current_height),
		kOxygenGaugeSize.x,
		current_height
	}.draw(gauge_color);

	// ゲージの枠線
	RectF{ kOxygenGaugePos, kOxygenGaugeSize }.drawFrame(2, 0, Palette::White);
}

void GameScene::DrawProgressMeter() const
{
	const double screen_height = Scene::Height();
	const double screen_right_x = Scene::Width() - kProgressMeterWidth;
	const double meter_center_x = screen_right_x + (kProgressMeterWidth / 2.0);

	const double total_travel = map_total_height_ - player_start_pos_.y;
	if(total_travel <= 0)
	{
		return;
	}

	RectF{ screen_right_x, 0, kProgressMeterWidth, screen_height }.draw(kUIGaugeBackgroundColor);

	Line{ meter_center_x, 0, meter_center_x, screen_height }.draw(1, kProgressLineColor);

	// Oxygenの位置を描画
	for(const auto& spot : oxygen_spots_)
	{
		double spot_y = 0.0;
		std::visit([&](const auto& shape)
				   {
					   // center() メンバ関数が存在する型のみアクセス
					   if constexpr(requires { shape.center(); })
					   {
						   spot_y = shape.center().y;
					   }
					   // center() がない型は何もしない
				   }, spot.GetCollider().shape);

		double spot_ratio = (spot_y - player_start_pos_.y) / total_travel;
		spot_ratio = Clamp(spot_ratio, 0.0, 1.0);

		const double marker_y = screen_height * spot_ratio;

		RectF{ Arg::center(meter_center_x, marker_y), kProgressSpotMarkerWidth, kProgressMarkerHeight }.draw(kProgressSpotColor);
	}

	double progress_ratio = (player_.GetPos().y - player_start_pos_.y) / total_travel;
	progress_ratio = Clamp(progress_ratio, 0.0, 1.0);

	const double marker_y = screen_height * progress_ratio;

	RectF{ Arg::center(meter_center_x, marker_y), kProgressPlayerMarkerWidth, kProgressMarkerHeight }.draw(kProgressPlayerColor);
}

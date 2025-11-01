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
	player_.Update(stage_);

	if((not is_player_dead_) && player_.IsOxygenEmpty())
	{
		is_player_dead_ = true;
		OnPlayerDied();
	}

	for(auto& spot : oxygen_spots_)
	{
		spot.Update();
	}

	// プレイヤーが死んだら，敵の更新や当たり判定をスキップ
	if(is_player_dead_)
	{
		if(kInputOK.down() || KeyEnter.down())
		{
			Vec2 respawn_pos = FindNearestRespawnSpot();

			// プレイヤー復活
			player_.Respawn(respawn_pos);

			// GameSceneの状態リセット
			is_player_dead_ = false;
		}

		// カメラは更新
		camera_manager_.SetTargetY(player_.GetPos().y);
		camera_manager_.Update();
		return;
	}

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
	for(auto& spot : oxygen_spots_)
	{
		spot.GetCollider().ClearCollisionResult();
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

	for(auto& spot : oxygen_spots_)
	{
		auto& spot_collider = spot.GetCollider();

		bool is_collided = std::visit([&](const auto& player_shape)
									  {
										  return std::visit([&](const auto& spot_shape)
															{
																return player_shape.intersects(spot_shape);
															}, spot_collider.shape);
									  }, player_collider.shape);

		if(is_collided)
		{
			// Player側にはOxygenと衝突したことを通知
			player_collider.is_colliding = true;
			player_collider.collided_tags.push_back(spot_collider.tag);

			// PlayerのOxygenを回復
			player_.RecoverOxygen();
		}
	}
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

	for(const auto& spot : oxygen_spots_)
	{
		spot.Draw(camera_offset);
	}

	// UI
	DrawOxygenGauge();
	DrawProgressMeter();
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

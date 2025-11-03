#include "../Core/AssetController.h"
#include "../Core/Config.h"
#include "../World/SpawnInfo.h"
#include "GameScene.h"

#include <Siv3D.hpp>
#include <variant>

GameScene::GameScene(const App::Scene::InitData& init)
	: IScene(init)
	, camera_manager_(
		(stage_.GetWidth()* stage_.GetTileSize()) / 2.0,
		kSceneSize
	)
	, map_total_height_(stage_.GetHeight()* stage_.GetTileSize())
{
	AssetController::GetInstance().PrepareAssets(U"Game");

	SpawnEntities();

	camera_manager_.SetTargetY(player_.GetPos().y);
	camera_manager_.SetYOffsetRatio(kTitleEndingCameraOffsetYRatio);

	//イントロBGMを再生開始（準備ができるまで保留して毎フレームチェック）
	StartOrDeferBGM(U"deepsea_intro", false);
}

GameScene::~GameScene()
{
	bgm_controller_.StopAll();
	AssetController::GetInstance().UnregisterAssets();
}

void GameScene::StartOrDeferBGM(const String& asset, bool loop)
{
	//既に現在再生中のBGMと同じなら何もしない
	if(not current_playing_bgm_asset_.isEmpty() && current_playing_bgm_asset_ == asset)
	{
		return;
	}

	// 即時再生できるなら再生して記録する
	if(AudioAsset::IsRegistered(asset) && AudioAsset::IsReady(asset))
	{
		bgm_controller_.Play(asset, loop);
		current_playing_bgm_asset_ = asset;
		// clear pending
		pending_bgm_asset_.clear();
		pending_bgm_loop_ = false;
		return;
	}

	//まだ準備できていない場合は保留して毎フレームチェックする
	pending_bgm_asset_ = asset;
	pending_bgm_loop_ = loop;
}

void GameScene::ProcessPendingBGM()
{
	if(pending_bgm_asset_.isEmpty())
	{
		return;
	}

	// 登録され準備完了したら再生する
	if(AudioAsset::IsRegistered(pending_bgm_asset_) && AudioAsset::IsReady(pending_bgm_asset_))
	{
		bgm_controller_.Play(pending_bgm_asset_, pending_bgm_loop_);
		current_playing_bgm_asset_ = pending_bgm_asset_;
		pending_bgm_asset_.clear();
		pending_bgm_loop_ = false;
	}
}

void GameScene::SpawnEntities()
{
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
			player_.SetPos(center_pos);
			player_start_pos_ = center_pos;
		}
		else if(info.type == U"Oxygen")
		{
			oxygen_spots_.emplace_back(center_pos, info.size);
		}
		else
		{
			enemies_.emplace_back(info.type, center_pos);
		}
	}
}

void GameScene::UpdateBGM()
{
	// pending 再生があれば毎フレームチェックして可能なら再生
	ProcessPendingBGM();

	//イントロが実際に再生されていて終了したらループBGMに切り替え
	if(not is_intro_finished_)
	{
		// intro が現在再生中か再生済みかを current_playing_bgm_asset_で管理
		if(current_playing_bgm_asset_ == U"deepsea_intro")
		{
			if(not bgm_controller_.IsPlaying(U"deepsea_intro"))
			{
				is_intro_finished_ = true;
				StartOrDeferBGM(U"deepsea", true);
			}
		}
		// else: introまだ準備できてない／再生されてない -> 待つ
	}

	// プレイヤーが死んだらBGMを停止
	if(player_.IsOxygenEmpty())
	{
		bgm_controller_.StopAll();
		current_playing_bgm_asset_.clear();
		pending_bgm_asset_.clear();
		pending_bgm_loop_ = false;
		return;
	}

	// 深度に応じて音量を調整
	const double total_travel = map_total_height_ - player_start_pos_.y;
	if(total_travel > 0)
	{
		double depth_ratio = (player_.GetPos().y - player_start_pos_.y) / total_travel;
		depth_ratio = Clamp(depth_ratio, 0.0, 1.0);

		// 深くなるほど音量を下げる
		const double volume = Math::Lerp(1.0, 0.0, depth_ratio);

		// 両方のBGMの音量を設定
		bgm_controller_.SetVolume(U"deepsea_intro", volume);
		bgm_controller_.SetVolume(U"deepsea", volume);
	}
}

void GameScene::update()
{
	// BGMの状態を更新
	UpdateBGM();

#if 0 // デバッグ用: 0 にすると無効化
	// Eキーでエンディング付近にワープ
	if(KeyE.down())
	{
		Vec2 current_pos = player_.GetPos();
		current_pos.y = 7600.0;
		player_.SetPos(current_pos);
		//Print << U"DEBUG: Warped to ending zone!";
	}
#endif

	switch(current_state_)
	{
	case GameState::Title:
	{
		for(auto& spot : oxygen_spots_)
		{
			spot.Update();
		}

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

		if(player_.GetPos().y >= kEndingZoneY)
		{
			current_state_ = GameState::Ending;

			const double camera_center_x = camera_manager_.GetViewRect().center().x;
			player_.StartEnding(camera_center_x);
			break;
		}

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
		player_.Update(stage_);
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

			// リスポーン時にBGMを再開
			is_intro_finished_ = false;
			StartOrDeferBGM(U"deepsea_intro", false);
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
	//Print << U"GAME SCENE: OXYGEN ZERO!";
}

Vec2 GameScene::FindNearestRespawnSpot() const
{
	const double dead_y = player_.GetPos().y;
	Vec2 best_spot_pos = Vec2::Zero();
	double min_distance = std::numeric_limits<double>::max();

	for(const auto& spot : oxygen_spots_)
	{
		const Vec2 spot_pos = spot.GetPos();

		if(spot_pos.y < dead_y)
		{
			const double distance_y = dead_y - spot_pos.y;
			if(distance_y < min_distance)
			{
				min_distance = distance_y;
				best_spot_pos = spot_pos;
			}
		}
	}

	if(best_spot_pos == Vec2::Zero())
	{
		return player_start_pos_;
	}

	return best_spot_pos;
}

void GameScene::draw() const
{
	static constexpr ColorF kSurfaceColor = kGameBackgroundColor;

	const double total_travel = map_total_height_ - player_start_pos_.y;
	double depth_ratio = 0.0;
	if(total_travel > 0)
	{
		depth_ratio = (player_.GetPos().y - player_start_pos_.y) / total_travel;
	}
	depth_ratio = Clamp(depth_ratio, 0.0, 1.0);

	const ColorF current_bg_color = kSurfaceColor.lerp(kDeepSeaColor, depth_ratio);
	Scene::SetBackground(current_bg_color);

	const Vec2 camera_offset = camera_manager_.GetCameraOffset();
	const RectF view_rect = camera_manager_.GetViewRect();

	// ヘルパー関数：背景を簡単に描画
	auto DrawBackground = [&](const String& texture_name, const Vec2& center_pos)
		{
			const Vec2 kDrawOffset = { 64.0, 64.0 };
			const Vec2 final_pos = s3d::Floor((center_pos - kDrawOffset) - camera_offset);
			TextureAsset(texture_name).draw(final_pos);
		};

	// ここで座標と画像名を指定するだけ
	DrawBackground(U"water_lay1", Vec2{ 50, 100 });
	DrawBackground(U"whale", Vec2{ 200, 400 });
	DrawBackground(U"jerry_fish", Vec2{ 200, 700 });
	DrawBackground(U"fish_02", Vec2{ 350, 1000 });
	DrawBackground(U"fish_01", Vec2{ 600, 1200 });
	DrawBackground(U"tuna", Vec2{ 300, 1500 });
	DrawBackground(U"fish_02", Vec2{ 500, 1600 });
	DrawBackground(U"tuna", Vec2{ 400, 1900 });
	DrawBackground(U"turtle", Vec2{ 400, 2000 });
	DrawBackground(U"fish_01", Vec2{ 600, 2500 });
	DrawBackground(U"stone-bream", Vec2{ 400, 2800 });
	DrawBackground(U"fish_02", Vec2{ 200, 2900 });
	DrawBackground(U"stone-bream", Vec2{ 600, 3300 });
	DrawBackground(U"sunfish", Vec2{ 250, 3500 });
	DrawBackground(U"fish_02", Vec2{ 450, 3700 });
	DrawBackground(U"stingray", Vec2{ 300, 4500 });
	DrawBackground(U"fish_02", Vec2{ 150, 4700 });
	DrawBackground(U"deapsea-fish01", Vec2{ 350, 5200 });
	DrawBackground(U"deapsea-fish02", Vec2{ 550, 6000 });
	DrawBackground(U"oarfish", Vec2{ 400, 7000 });
	DrawBackground(U"chair", Vec2{ 550, 7200 });
	DrawBackground(U"sofa", Vec2{ 400, 7500 });
	DrawBackground(U"deapsea-fish01", Vec2{ 150, 7350 });
	DrawBackground(U"TV1", Vec2{ 100, 7800 });

	// プレイヤー開始位置にtitleを描画
	if(TextureAsset::IsRegistered(U"title"))
	{
		const Vec2 title_world_pos = player_start_pos_;
		Vec2 title_screen_pos = title_world_pos - camera_offset;
		title_screen_pos += Vec2{ -330.0, -400.0 }; // 少し上にオフセット
		TextureAsset(U"title").draw(title_screen_pos);
	}

	// エンディング座標にoctopusを描画（背景の直後、他のオブジェクトより前）
	if(TextureAsset::IsRegistered(U"octopus"))
	{
		const Vec2 octopus_world_pos = Vec2{ stage_.GetWidth() * stage_.GetTileSize() / 2.0, 7300.0 };
		const Vec2 octopus_screen_pos = octopus_world_pos - camera_offset;
		TextureAsset(U"octopus").drawAt(octopus_screen_pos);
	}

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
}

void GameScene::DrawOxygenGauge() const
{
	RectF{ kOxygenGaugePos, kOxygenGaugeSize }.draw(kUIGaugeBackgroundColor);

	const double current_oxygen = player_.GetOxygen();
	const double max_oxygen = player_.GetMaxOxygen();

	const double oxygen_ratio = (current_oxygen / max_oxygen);

	ColorF gauge_color;
	if(current_oxygen > kOxygenWarningThreshold)
	{
		gauge_color = kOxygenColorSafe;
	}
	else if(current_oxygen > kOxygenDangerThreshold)
	{
		gauge_color = kOxygenColorWarning;
	}
	else
	{
		gauge_color = kOxygenColorDanger;
	}

	const double current_height = kOxygenGaugeSize.y * oxygen_ratio;

	RectF{
		kOxygenGaugePos.x,
		kOxygenGaugePos.y + (kOxygenGaugeSize.y - current_height),
		kOxygenGaugeSize.x,
		current_height
	}.draw(gauge_color);

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

	for(const auto& spot : oxygen_spots_)
	{
		double spot_y = 0.0;
		std::visit([&](const auto& shape)
				   {
					   if constexpr(requires { shape.center(); })
					   {
						   spot_y = shape.center().y;
					   }
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

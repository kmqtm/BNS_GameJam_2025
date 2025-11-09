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

			// エンディング開始時刻を記録
			ending_start_time_ = Scene::Time();

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

		// CollisionManagerを使用した衝突判定（O(N)に最適化）
		collision_manager_.ClearResults();

		// Playerのコライダーを登録
		collision_manager_.RegisterPlayer(&player_.collider);

		// 敵のコライダーを登録（生存中のもののみ）
		for(size_t i = 0; i < enemies_.size(); ++i)
		{
			if(enemies_[i].IsAlive())
			{
				collision_manager_.RegisterOther(&enemies_[i].GetCollider(), static_cast<uint32_t>(i + 1));
			}
		}

		// 酸素スポットのコライダーを登録
		const uint32_t oxygen_id_offset = static_cast<uint32_t>(enemies_.size() + 1);
		for(size_t i = 0; i < oxygen_spots_.size(); ++i)
		{
			collision_manager_.RegisterOther(&oxygen_spots_[i].GetCollider(), oxygen_id_offset + static_cast<uint32_t>(i));
		}

		// Player vs Other の衝突判定を実行（O(N)）
		collision_manager_.ResolveCollisions();

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

			// エンディングタイマーをリセット
			ending_start_time_ = -1.0;
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

	// ヘルパー関数：背景を簡単に描画（プレイヤーの近くにいる場合のみ）
	const double render_distance = stage_.GetTileSize() * 12; // 12マス分の距離
	const Vec2 player_pos = player_.GetPos();

	auto DrawBackground = [&](const String& texture_name, const Vec2& center_pos, bool isFlip = false, const Vec2& velocity = Vec2{ 0.0, 0.0 }, bool isWave = false)
		{
			// 各背景オブジェクトを識別するためのユニークキーを生成
			const String unique_key = U"{}_{:.1f}_{:.1f}"_fmt(texture_name, center_pos.x, center_pos.y);

			// プレイヤーとの距離をチェック（初期位置で）
			const double distance = player_pos.distanceFrom(center_pos);

			// プレイヤーが範囲内に入った場合、アクティブ化時刻を記録
			if(distance <= render_distance)
			{
				if(background_activation_times_.find(unique_key) == background_activation_times_.end())
				{
					// まだ記録されていない場合、現在時刻を記録
					background_activation_times_[unique_key] = Scene::Time();
				}
			}

			// アクティブ化されていない場合は、元の位置に描画
			Vec2 animated_pos = center_pos;

			// アクティブ化されている場合のみ、移動を計算
			if(background_activation_times_.find(unique_key) != background_activation_times_.end())
			{
				const double activation_time = background_activation_times_[unique_key];
				const double elapsed_time = Scene::Time() - activation_time;

				// アクティブ化時刻からの経過時間に応じて速度分だけ移動
				animated_pos.x += elapsed_time * velocity.x;
				animated_pos.y += elapsed_time * velocity.y;

				// 波の揺れを追加
				if(isWave)
				{
					const double wave_amplitude = 1.0; // 揺れの振幅（ピクセル）
					const double wave_frequency = 1.0; // 揺れの速さ
					animated_pos.y += Math::Sin(elapsed_time * wave_frequency) * wave_amplitude;
				}
			}

			// 描画範囲外の場合はスキップ
			if(distance > render_distance)
			{
				return;
			}

			const Vec2 kDrawOffset = { 64.0, 64.0 };
			const Vec2 final_pos = s3d::Floor((animated_pos - kDrawOffset) - camera_offset);

			// 左右反転して描画
			if(isFlip)
			{
				TextureAsset(texture_name).mirrored().draw(final_pos);
			}
			else
			{
				TextureAsset(texture_name).draw(final_pos);
			}
		};

	// ここで座標と画像名を指定するだけ
	DrawBackground(U"water_lay1", Vec2{ 50, 100 }, false);
	DrawBackground(U"whale", Vec2{ 300, 400 }, false, Vec2{ -10, 0 });
	DrawBackground(U"jerry_fish", Vec2{ 200, 700 }, false, Vec2{ 5, -20 });
	DrawBackground(U"tuna", Vec2{ 200, 950 }, true, Vec2{ 40, 0 });
	DrawBackground(U"fish_02", Vec2{ 600, 1000 }, false, Vec2{ -30, 0 }, true);
	DrawBackground(U"fish_01", Vec2{ 250, 1300 }, true, Vec2{ 30, 0 });
	DrawBackground(U"tuna", Vec2{ 700, 1500 }, false, Vec2{ -40, 0 });
	DrawBackground(U"fish_02", Vec2{ 700, 1600 }, false, Vec2{ -30, 0 }, true);
	DrawBackground(U"fish_01", Vec2{ 100, 1750 }, true, Vec2{ 20, 0 }, true);
	DrawBackground(U"turtle", Vec2{ 200, 2150 }, true, Vec2{ 15, -10 });
	DrawBackground(U"fish_02", Vec2{ 650, 2000 }, false, Vec2{ -30, 0 }, true);
	DrawBackground(U"fish_01", Vec2{ 700, 2500 }, false, Vec2{ -20, 0 });
	DrawBackground(U"stone-bream", Vec2{ 700, 2800 }, false, Vec2{ -20, 0 });
	DrawBackground(U"fish_02", Vec2{ 100, 2900 }, true, Vec2{ 20, 0 }, true);
	DrawBackground(U"stone-bream", Vec2{ 600, 3300 }, false, Vec2{ -20, 0 });
	DrawBackground(U"fish_02", Vec2{ 600, 3600 }, false, Vec2{ -20, 0 }, true);
	DrawBackground(U"sunfish", Vec2{ 200, 3900 }, false, Vec2{ 5, 0 });
	DrawBackground(U"stingray", Vec2{ 200, 4600 }, false, Vec2{ 5, -5 });
	DrawBackground(U"stone-bream", Vec2{ 100, 5000 }, true, Vec2{ 10, 0 });
	DrawBackground(U"deepsea-fish01", Vec2{ 200 , 5400 }, true, Vec2{ 5, 0 });
	DrawBackground(U"deepsea-fish03", Vec2{ 500, 5700 }, true, Vec2{ -5, 0 });
	DrawBackground(U"chair", Vec2{ 200, 5900 }, false, Vec2{ 0, -10 });
	DrawBackground(U"deepsea-fish02", Vec2{ 100, 6475 });
	DrawBackground(U"oarfish", Vec2{ 600, 6500 }, false, Vec2{ -5, -5 });
	DrawBackground(U"sofa", Vec2{ 500, 6700 }, false, Vec2{ 0, -10 });
	DrawBackground(U"TV1", Vec2{ 200, 7000 }, false, Vec2{ 0, -10 });
	DrawBackground(U"guide_text1", Vec2{ 220, 990 });
	DrawBackground(U"guide_text2", Vec2{ 500, 990 });
	DrawBackground(U"guide_text3", Vec2{ 230, 1400 });
	DrawBackground(U"guide_text4", Vec2{ 250, 2370 });

	// プレイヤー開始位置にtitleを描画
	if(TextureAsset::IsRegistered(U"title"))
	{
		const Vec2 title_world_pos = player_start_pos_;
		Vec2 title_screen_pos = title_world_pos - camera_offset;
		title_screen_pos += Vec2{ -330.0, -400.0 }; // 少し上にオフセット
		TextureAsset(U"title").draw(title_screen_pos);
	}

	// エンディング座標にoctopusを描画（背景の直後、他のオブジェクトより前）
	{
		// エンディング開始から8.4秒後に笑顔へ切り替え
		const bool showSmile = (current_state_ == GameState::Ending)
			&& (ending_start_time_ >= 0.0)
			&& ((Scene::Time() - ending_start_time_) >= kOctopusSmileDelay);

		const String texName = showSmile ? U"octopus_smile" : U"octopus";

		if(TextureAsset::IsRegistered(texName))
		{
			const Vec2 octopus_world_pos = Vec2{ stage_.GetWidth() * stage_.GetTileSize() / 2.0,7300.0 };
			const Vec2 octopus_screen_pos = octopus_world_pos - camera_offset;
			TextureAsset(texName).drawAt(octopus_screen_pos);
		}

		// 笑顔になった後に画面を暗くしオーバレイ画像を描画
		if(showSmile)
		{
			// showSmile になってからの経過時間を計算
			const double smile_elapsed_time = Scene::Time() - ending_start_time_ - kOctopusSmileDelay;

			// 0.8 秒以上経過してから画面を薄暗くする
			if(smile_elapsed_time >= 0.8)
			{
				//画面全体を薄暗く
				Rect{ 0, 0, Scene::Width(), Scene::Height() }.draw(ColorF{ 0, 0, 0, kEndingDarkenAlpha });

				// オーバレイ画像が存在すれば中央より少し上に描画
				if(TextureAsset::IsRegistered(kEndingOverlayTexture))
				{
					constexpr int overlayYOffset = -190; // 少し上に
					TextureAsset(kEndingOverlayTexture).drawAt(Scene::Center().movedBy(0, overlayYOffset));
				}
			}
		}
	}

	player_.Draw(camera_offset);

	for(const auto& enemy : enemies_)
	{
		enemy.Draw(camera_offset);
	}

	stage_.Draw(camera_offset, view_rect);

	for(const auto& spot : oxygen_spots_)
	{
		spot.Draw(camera_offset);
	}

	DrawOxygenGauge();
	DrawProgressMeter();

	if(current_state_ == GameState::Title)
	{
		DrawBackground(U"title_text", Vec2{ stage_.GetWidth() * stage_.GetTileSize() / 2.0 - 100, 600 });
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

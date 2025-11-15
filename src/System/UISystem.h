#pragma once

/// @file UISystem.h
/// @brief UI表示管理システムの定義

#include <Siv3D.hpp>

/// @brief UIの描画を一元管理する
class UISystem
{
public:
	/// @brief コンストラクタ
	UISystem() = default;

	/// @brief UIの表示情報を更新する
	/// @param current_oxygen 現在の酸素量
	/// @param max_oxygen 最大酸素量
	/// @param player_y プレイヤーのY座標
	/// @param start_y スタート位置のY座標
	/// @param total_height ステージの総高さ
	/// @param spot_positions 酸素スポット位置のリスト
	void Update(double current_oxygen,
				double max_oxygen,
				double player_y,
				double start_y,
				double total_height,
				const s3d::Array<s3d::Vec2>& spot_positions);

	/// @brief UIを描画する
	void Draw() const;

private:
	/// @brief 酸素ゲージを描画する
	void DrawOxygenGauge() const;

	/// @brief 進捗メーターを描画する
	void DrawProgressMeter() const;

	/// @brief 現在の酸素量
	double current_oxygen_ = 0.0;

	/// @brief 最大酸素量
	double max_oxygen_ = 1.0;

	/// @brief プレイヤーのY座標
	double player_y_ = 0.0;

	/// @brief スタート位置のY座標
	double start_y_ = 0.0;

	/// @brief ステージの総高さ
	double total_height_ = 0.0;

	/// @brief 酸素スポット位置のリスト
	s3d::Array<s3d::Vec2> spot_positions_;
};

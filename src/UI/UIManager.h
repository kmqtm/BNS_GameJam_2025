#pragma once
#include <Siv3D.hpp>

// UI 表示を一元管理するマネージャ
class UIManager
{
public:
	UIManager() = default;

	// 表示に必要な情報を受け取って内部に保持
	void Update(double current_oxygen,
				double max_oxygen,
				double player_y,
				double start_y,
				double total_height,
				const s3d::Array<s3d::Vec2>& spot_positions);

	// UI の描画
	void Draw() const;

private:
	void DrawOxygenGauge() const;
	void DrawProgressMeter() const;

	// Update で受け取った値
	double current_oxygen_ = 0.0;
	double max_oxygen_ = 1.0; // 0除算回避
	double player_y_ = 0.0;
	double start_y_ = 0.0;
	double total_height_ = 0.0;
	s3d::Array<s3d::Vec2> spot_positions_;
};

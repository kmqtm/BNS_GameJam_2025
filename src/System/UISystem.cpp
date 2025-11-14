#include "UISystem.h"
#include <Siv3D.hpp>

namespace
{
	using namespace s3d;

	// UI 定数（GameScene から移設）
	constexpr Vec2  kOxygenGaugePos{ 20, 20 };
	constexpr Size  kOxygenGaugeSize{ 24, 200 };
	constexpr ColorF kUIGaugeBackgroundColor{ 0.0, 0.5 };

	constexpr ColorF kOxygenColorSafe = Palette::Limegreen;
	constexpr ColorF kOxygenColorWarning = Palette::Yellow;
	constexpr ColorF kOxygenColorDanger = Palette::Red;

	constexpr double kOxygenWarningThreshold = 70.0;
	constexpr double kOxygenDangerThreshold = 30.0;

	constexpr double kProgressMeterWidth = 10.0;
	constexpr ColorF kProgressLineColor = Palette::White;
	constexpr ColorF kProgressPlayerColor = Palette::Yellow;
	constexpr ColorF kProgressSpotColor = Palette::Limegreen;
	constexpr double kProgressPlayerMarkerWidth = 14.0;
	constexpr double kProgressSpotMarkerWidth = 8.0;
	constexpr double kProgressMarkerHeight = 4.0;
}

void UISystem::Update(double current_oxygen,
					  double max_oxygen,
					  double player_y,
					  double start_y,
					  double total_height,
					  const s3d::Array<s3d::Vec2>& spot_positions)
{
	current_oxygen_ = current_oxygen;
	max_oxygen_ = (max_oxygen > 0.0) ? max_oxygen : 1.0;
	player_y_ = player_y;
	start_y_ = start_y;
	total_height_ = total_height;
	spot_positions_ = spot_positions;
}

void UISystem::Draw() const
{
	DrawOxygenGauge();
	DrawProgressMeter();
}

void UISystem::DrawOxygenGauge() const
{
	RectF{ kOxygenGaugePos, kOxygenGaugeSize }.draw(kUIGaugeBackgroundColor);

	const double oxygen_ratio = current_oxygen_ / max_oxygen_;

	ColorF gauge_color;
	if(current_oxygen_ > kOxygenWarningThreshold)
	{
		gauge_color = kOxygenColorSafe;
	}
	else if(current_oxygen_ > kOxygenDangerThreshold)
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

void UISystem::DrawProgressMeter() const
{
	const double screen_height = Scene::Height();
	const double screen_right_x = Scene::Width() - kProgressMeterWidth;
	const double meter_center_x = screen_right_x + (kProgressMeterWidth / 2.0);

	const double total_travel = total_height_ - start_y_;
	if(total_travel <= 0.0)
	{
		return;
	}

	// 背景
	RectF{ screen_right_x, 0, kProgressMeterWidth, screen_height }.draw(kUIGaugeBackgroundColor);
	Line{ meter_center_x, 0, meter_center_x, screen_height }.draw(1, kProgressLineColor);

	// 酸素スポットマーカー
	for(const auto& pos : spot_positions_)
	{
		double spot_ratio = (pos.y - start_y_) / total_travel;
		spot_ratio = Clamp(spot_ratio, 0.0, 1.0);
		const double marker_y = screen_height * spot_ratio;

		RectF{ Arg::center(meter_center_x, marker_y), kProgressSpotMarkerWidth, kProgressMarkerHeight }.draw(kProgressSpotColor);
	}

	// プレイヤーマーカー
	double progress_ratio = (player_y_ - start_y_) / total_travel;
	progress_ratio = Clamp(progress_ratio, 0.0, 1.0);
	const double player_marker_y = screen_height * progress_ratio;

	RectF{ Arg::center(meter_center_x, player_marker_y), kProgressPlayerMarkerWidth, kProgressMarkerHeight }.draw(kProgressPlayerColor);
}

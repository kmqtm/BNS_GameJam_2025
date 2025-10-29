#pragma once
# include <Siv3D.hpp>

namespace Core
{
	class CameraManager
	{
	public:
		CameraManager(double fixed_world_x, const Size& view_size);

		void SetTargetY(double target_y);

		void Update();
		Vec2 GetCameraOffset() const;
		RectF GetViewRect() const;

	private:
		Camera2D camera_{ Vec2::Zero(), 1.0, Camera2DParameters::NoControl() };

		// 固定するX座標と，Y軸のオフセット値
		double fixed_world_x_;
		double y_offset_;

		Size view_size_;

		// ターゲットのY座標と，現在のカメラのY座標
		double target_y_ = 0.0;
		double current_y_ = 0.0;
	};
}

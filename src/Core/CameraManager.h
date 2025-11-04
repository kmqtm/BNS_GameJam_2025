#pragma once
# include <Siv3D.hpp>

class CameraManager
{
public:
	CameraManager(double fixed_world_x, const Size& view_size);

	void SetTargetY(double target_y);

	// カメラのY軸オフセット比率を変更する
	// (例: 1.0/6.0 = 上1/3, -1.0/6.0 = 下1/3)
	void SetYOffsetRatio(double ratio);

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

	// カメラ中心の目標Yを計算する（target_y_ + y_offset_）
	double ComputeGoalY() const;

	// 現在値を目標に滑らかに補間する
	double SmoothTo(double current, double goal, double factor) const;

	// カメラ中心座標を作成する
	Vec2 ComputeCameraCenter() const;

	// ビューサイズの半分を返す（オフセット計算に使用）
	Vec2 HalfViewSize() const;
};


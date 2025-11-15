#pragma once

/// @file CameraSystem.h
/// @brief カメラ管理システムの定義

# include <Siv3D.hpp>

/// @brief 2Dカメラを管理する
class CameraSystem
{
public:
	/// @brief コンストラクタ
	/// @param fixed_world_x カメラのX座標(固定)
	/// @param view_size ビューサイズ
	CameraSystem(double fixed_world_x, const Size& view_size);

	/// @brief カメラのターゲットY座標を設定する
	/// @param target_y ターゲットY座標
	void SetTargetY(double target_y);

	/// @brief カメラのY軸オフセット比率を変更する
	/// @param ratio オフセット比率(例:1.0/6.0で上1/3,マイナス値で下方向)
	void SetYOffsetRatio(double ratio);

	/// @brief カメラを更新する
	void Update();

	/// @brief カメラオフセットを取得する
	/// @return カメラオフセット
	Vec2 GetCameraOffset() const;

	/// @brief ビュー矩形を取得する
	/// @return ビュー矩形
	RectF GetViewRect() const;

private:
	/// @brief 2Dカメラ
	Camera2D camera_{ Vec2::Zero(), 1.0, Camera2DParameters::NoControl() };

	/// @brief X座標(固定)
	double fixed_world_x_;

	/// @brief Y軸のオフセット値
	double y_offset_;

	/// @brief ビューサイズ
	Size view_size_;

	/// @brief ターゲットのY座標
	double target_y_ = 0.0;

	/// @brief 現在のカメラのY座標
	double current_y_ = 0.0;

	/// @brief カメラ中心の目標Yを計算する
	/// @return 計算された目標Y(target_y_ + y_offset_)
	double ComputeGoalY() const;

	/// @brief 現在値を目標に滑らかに補間する
	/// @param current 現在値
	/// @param goal 目標値
	/// @param factor 補間係数
	/// @return 補間された値
	double SmoothTo(double current, double goal, double factor) const;

	/// @brief カメラ中心座標を作成する
	/// @return カメラ中心座標
	Vec2 ComputeCameraCenter() const;

	/// @brief ビューサイズの半分を取得する
	/// @return ビューサイズの半分
	Vec2 HalfViewSize() const;
};


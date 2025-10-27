#pragma once
#include "Utility.h" // Utility::RoundVec2 のためにインクルード
#include <Siv3D.hpp>

namespace Core
{
	class CameraManager
	{
	public:
		CameraManager();

		void Update();

		// カメラの追従ターゲットを設定
		void SetTarget(const s3d::Vec2& target);

		// 描画用のピクセルパーフェクトなオフセットを取得
		s3d::Vec2 GetSnappedOffset() const;

		// 描画カリング用のビュー矩形を取得
		s3d::RectF GetViewRect() const;

	private:
		s3d::Camera2D camera_;
	};
}

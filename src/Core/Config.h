#pragma once

/// @file Config.h
/// @brief ゲーム設定の定義

#include <Siv3D.hpp>

/// @brief ゲームシーンのサイズ(ピクセル)
inline constexpr Size kSceneSize = { 704, 900 };

/// @brief メニューシーンの背景色
inline constexpr ColorF kBackgroundColor = ColorF{ 0.8, 0.9, 1.0 };

/// @brief ゲームシーンの背景色
inline constexpr ColorF kGameBackgroundColor = ColorF{ 0.4, 0.7, 1.0 };

/// @brief シーンを識別するID
enum class SceneID : int8
{
	kTitle = 0,  ///< タイトルシーン
	kGame,       ///< ゲームシーン
};

/// @brief Siv3DのシーンマネージャーにSceneIDを適用した型
using App = s3d::SceneManager<SceneID>;

/// @brief 決定キー
const InputGroup kInputOK{ KeyEnter };

/// @brief キャンセルキー
const InputGroup kInputCancel{ KeyEscape };

/// @brief 左移動キー
const InputGroup kInputLeft{ KeyLeft, KeyA };

/// @brief 右移動キー
const InputGroup kInputRight{ KeyRight, KeyD };

/// @brief 上移動キー
const InputGroup kInputUp{ KeyUp, KeyW };

/// @brief 下移動キー
const InputGroup kInputDown{ KeyDown, KeyS };

/// @brief アクション1キー
const InputGroup kInputAction1{ KeySpace };

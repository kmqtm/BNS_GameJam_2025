#pragma once

#include <Siv3D.hpp>

// シーンの基本サイズ
inline constexpr Size kSceneSize = {240, 300};
// デバッグ用の背景色(白)
const ColorF kBackgroundColor = ColorF{ 0.8, 0.9, 1.0 };
// ゲーム用の背景色(水色)
const ColorF kGameBackgroundColor = ColorF{ 0.4, 0.7, 1.0 };

// シーンを識別するためのID
enum class SceneID : int8 {
	Title = 0,
	Game,
};
using App = s3d::SceneManager<SceneID>;

// 入力キーの定義
const InputGroup kInputOK{KeyEnter};
const InputGroup kInputCancel{KeyEscape};
const InputGroup kInputLeft{KeyLeft, KeyA};
const InputGroup kInputRight{KeyRight, KeyD};
const InputGroup kInputUp{KeyUp, KeyW};
const InputGroup kInputDown{KeyDown, KeyS};

#pragma once

#include <Siv3D.hpp>

inline constexpr Size kSceneSize = { 240, 300 };
const ColorF kBackgroundColor = ColorF{ 0.8, 0.9, 1.0 };
const ColorF kGameBackgroundColor = ColorF{ 0.4, 0.7, 1.0 };

enum class SceneID : int8
{
	kTitle = 0,
	kGame,
};
using App = s3d::SceneManager<SceneID>;

const InputGroup kInputOK{ KeyEnter };
const InputGroup kInputCancel{ KeyEscape };
const InputGroup kInputLeft{ KeyLeft, KeyA };
const InputGroup kInputRight{ KeyRight, KeyD };
const InputGroup kInputUp{ KeyUp, KeyW };
const InputGroup kInputDown{ KeyDown, KeyS };
#include "ScreenManager.h"
#include "Config.h"

#include <Siv3D.hpp>

namespace ScreenManager {
	void Initialize() {
		// ウィンドウのタイトルを設定
		Window::SetTitle(U"BNS GameJam 2025");

		// ウィンドウをリサイズ可能にする
		Window::SetStyle(WindowStyle::Sizable);

		// シーンの基本サイズでウィンドウを作成
		Window::Resize(kSceneSize);

		// シーンのアスペクト比を保つためのレターボックスを設定(黒色)
		Scene::SetLetterbox(ColorF{ 0, 0, 0 });

		Scene::SetResizeMode(ResizeMode::Keep);
	}
}

#include "Config.h"
#include "ScreenManager.h"

#include <Siv3D.hpp>

namespace ScreenManager
{
	void Initialize()
	{
		// ウィンドウのタイトルを設定
		Window::SetTitle(U"BNS GameJam 2025");

		// ウィンドウをリサイズ可能にする
		Window::SetStyle(WindowStyle::Sizable);

		// シーン転送時の拡大縮小方法を最近傍法にする
		Scene::SetTextureFilter(TextureFilter::Nearest);

		// シーンの基本サイズでウィンドウを作成
		Window::Resize(kSceneSize);

		// シーンのアスペクト比を保つためのレターボックスを設定(黒色)
		Scene::SetLetterbox(ColorF{ 0, 0, 0 });

		Scene::SetResizeMode(ResizeMode::Keep);

		//Window::Resize(kSceneSize * 2);
	}
}

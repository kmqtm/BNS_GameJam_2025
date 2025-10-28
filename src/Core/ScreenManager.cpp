#include "Config.h"
#include "ScreenManager.h"

#include <Siv3D.hpp>

// (命名) namespace を lower_case に修正
namespace screen_manager
{
	void Initialize()
	{
		Window::SetTitle(U"BNS GameJam 2025");
		Window::SetStyle(WindowStyle::Sizable);
		Scene::SetTextureFilter(TextureFilter::Nearest);
		Window::Resize(kSceneSize);
		Scene::SetLetterbox(ColorF{ 0, 0, 0 });
		Scene::SetResizeMode(ResizeMode::Keep);
		Window::Maximize();
	}
}

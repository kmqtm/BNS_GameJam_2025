#include "Core/Config.h"
#include "Scenes/GameScene.h"

#include <Siv3D.hpp>

void Main()
{
	// ウィンドウの初期設定
	Window::SetTitle(U"BNS GameJam 2025");
	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetTextureFilter(TextureFilter::Nearest);
	Window::Resize(kSceneSize);
	Scene::SetLetterbox(ColorF{ 0, 0, 0 });
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::Maximize();

	// シーンマネージャーを作成
	App manager;
	manager.add<GameScene>(SceneID::kGame);
	manager.init(SceneID::kGame, 10.0ms);

	// ウィンドウを閉じるユーザアクションのみを終了操作に設定
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	while(System::Update())
	{
		if(not manager.update())
		{
			break;
		}
	}
}

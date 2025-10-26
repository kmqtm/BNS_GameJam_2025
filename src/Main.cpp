#include "Core/Config.h"
#include "Core/ScreenManager.h"
#include "Scenes/GameScene.h"

#include <Siv3D.hpp>

void Main()
{
	// ウィンドウの初期設定を一括で行う
	ScreenManager::Initialize();

	// シーンマネージャーを作成
	App manager;
	manager.add<GameScene>(SceneID::Game);
	manager.init(SceneID::Game, 10.0ms);

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

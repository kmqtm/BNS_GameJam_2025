#pragma once

#include "../Core/Config.h"

#include <Siv3D.hpp>

// メインのゲームプレイシーン
class GameScene : public App::Scene {
public:
	GameScene(const InitData &init);
	~GameScene() override;

	void update() override;
	void draw() const override;

private:
};

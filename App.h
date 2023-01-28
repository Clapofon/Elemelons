#pragma once

#include "../Engine/IMainGame.h"

#include "GameplayScene.h"
#include "MainMenuScene.h"

class App : public Engine::IMainGame
{
public:
	App();
	~App();

	virtual void onInit() override;
	virtual void addScreens() override;
	virtual void onExit() override;
private:
	std::unique_ptr<GameplayScene> m_gameplayScene = nullptr;
	std::unique_ptr<MainMenuScene> m_mainMenuScene = nullptr;
};


#pragma once

#include <SDL.h>

#include "../Engine/IGameScene.h"
#include "../Engine/IMainGame.h"
#include "../Engine/Camera.h"

enum class Menu
{
	MAIN,
	SETTINGS
};

enum class Setting
{
	AUDIO,
	VIDEO,
	GENERAL
};

class MainMenuScene : public Engine::IGameScene
{
public:
	MainMenuScene();
	~MainMenuScene();

	int getNextScreenIndex() const override;
	int getPreviousScreenIndex() const override;

	void build() override;
	void destroy() override;

	void onEntry() override;
	void onExit() override;

	void update(double deltaTime) override;
	void draw() override;

	void handleInput(double deltaTime);

private:

	void drawMainMenu();
	void drawSettingsMenu();

	void drawGeneralSettings(const glm::vec2& size);
	void drawAudioSettings(const glm::vec2& size);
	void drawVideoSettings(const glm::vec2& size);

	void applySettings();

	void setupStyle();

	void preloadTextures();

	Menu m_currentMenu = Menu::MAIN;
	Setting m_currentSetting = Setting::AUDIO;
	std::string m_selectedResolution;
	std::string m_selectedWindowMode = "windowed";



	bool m_madeChanges = false;
};


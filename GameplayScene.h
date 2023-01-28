#pragma once

#include "../Engine/IGameScene.h"
#include "../Engine/Camera.h"
#include "../Engine/GLSLProgram.h"
#include "../Engine/SpriteBatch.h"
#include "../Engine/LineRenderer.h"
#include "../Engine/Light.h"
#include "../Engine/FrameBuffer.h"

#include "Crate.h"
#include "Player.h"
#include "World.h"
#include "GameplayData.h"

#include <Box2D/Box2D.h>

#include <sstream>



class GameplayScene : public Engine::IGameScene
{
public:
	GameplayScene();
	~GameplayScene();

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

	void drawUI();

	void drawBonfireMenu(Player& player);
	void drawLevelUpMenu(Player& player);
	void drawInventoryMenu(Player& player);
	void drawTravelMenu(Player& player);

	void drawMainMenu(Player& player);

	void drawHUD(Player& player);
	void drawCrosshair();

	void drawDebugWindow(Player& player);

	void AStarTest();
	void setupStyle();

	uint32_t getLevelUpCost(uint32_t level);

	Engine::Camera m_camera;
	Engine::ParticleSystem2D m_particleSystem;

	GameplayState m_gameplayState = GameplayState::PLAY;

	glm::vec2 m_screenDims = glm::vec2(0);

	std::stringstream m_statStream;

	std::unique_ptr<b2World> m_b2World;
	Map m_level;
	std::string m_regionSelected = "Starting Area";

	Engine::LineRenderer m_lineRenderer;

	int32_t m_ambientChannelId = -1;

	PlayerData m_playerData;
	PlayerData m_initialPlayerData;
	uint32_t m_reqExp;

	bool m_menuDisplay = false;
	bool m_inventoryDisplay = false;
	bool m_drawCrosshair = true;

	Engine::Light m_testLight;

	float m_loadingIconAngle = 0.0f;

	uint32_t m_detailItem = 0;
	uint32_t m_selectedBonfire = 0;

	Engine::FrameBuffer m_framebuffer;
};
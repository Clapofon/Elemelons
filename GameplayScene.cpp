#include "GameplayScene.h"

#include "../Engine/IMainGame.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Random.h"
#include "../Engine/Renderer2D.h"
#include "../Engine/StringUtil.h"
#include "../Engine/Constants.h"
#include "../Engine/PathFinding.h"
#include "SaveManager.h"

#include "Indices.h"

#include <SDL.h>
#include <glew.h>

#include <iostream>
#include <filesystem>
#include <fstream>

#include <imgui.h>

GameplayScene::GameplayScene()
{
	m_sceneIndex = SCENE_INDEX_GAMEPLAY_SCENE;
}

GameplayScene::~GameplayScene()
{

}

int GameplayScene::getNextScreenIndex() const
{
	return SCENE_INDEX_NO_SCENE;
}

int GameplayScene::getPreviousScreenIndex() const
{
	return SCENE_INDEX_NO_SCENE;
}

void GameplayScene::build()
{
	
}

void GameplayScene::destroy()
{
	
}

void GameplayScene::onEntry()
{
	//world
	b2Vec2 gravity(0.0f, -9.81f);
	m_b2World = std::make_unique<b2World>(gravity);

	m_screenDims = m_game->getWindowDimentions();
	m_camera.init(m_screenDims);
	m_camera.setScale(1.0f);
	m_camera.setPosition(glm::vec2(0, 0));
	m_particleSystem.init(&m_camera);

	m_camera.update(m_screenDims);

	//m_player.setConfig(m_game->getPlayerConfig());

	/*
		Add all your batches firts!
		If not, the batches wont get inititialized!
		Then you can call Renderer2D::Init()
	*/
	Engine::Renderer2D::AddBatch("staticBG");
	Engine::Renderer2D::AddBatch("staticBGTrees");
	Engine::Renderer2D::AddBatch("staticFG");
	Engine::Renderer2D::AddBatch("staticFGNear");
	Engine::Renderer2D::AddBatch("tmxLevel");
	Engine::Renderer2D::AddBatch("dynamic");
	Engine::Renderer2D::AddBatch("debugText");
	Engine::Renderer2D::AddBatch("text");
	Engine::Renderer2D::AddBatch("staticLevelObjects");
	Engine::Renderer2D::AddBatch("hud");
	Engine::Renderer2D::AddBatch("framebuffer");
	Engine::Renderer2D::Init(&m_camera);

	m_level.init("assets/maps/map1.tmx", { -12800, 6064 }, 4.0f, m_b2World.get(), m_game, "tmxLevel", &m_camera);
	m_level.drawStatic();

	m_lineRenderer.init();

	m_game->setFrameLock(true);

	m_game->getAudioEngine().LoadSound("assets/audio/ambient/forest_ambient.wav", true, true, true);
	m_ambientChannelId = m_game->getAudioEngine().playSound("assets/audio/ambient/forest_ambient.wav", { 0, 0, 0 }, -20.0f);
	AStarTest();
	setupStyle();

	m_framebuffer.init(m_screenDims, m_screenDims);

	//m_testLight.init({ 0, 0, 0 }, { 255, 0, 0 });
}

void GameplayScene::onExit()
{
	m_game->getAudioEngine().StopAllChannels();
	m_game->getAudioEngine().ResetPositionAllChannels();

	//m_level.save(Type::DEFFERED);
}

void GameplayScene::update(double deltaTime)
{
	m_screenDims = m_game->getWindowDimentions();
	m_framebuffer.update(m_screenDims, m_screenDims);

	if (m_game->getWindow().wasResized())
	{
		printf("dims: %f, %f\n", m_screenDims.x, m_screenDims.y);
		m_camera.init(m_screenDims);
		m_game->getWindow().setResize(false);
	}

	handleInput(deltaTime);


	glm::vec2 playerPos = m_level.getPlayer().getPosition();

	m_testLight.setPosition({ playerPos.x, playerPos.y, 0 });

	m_camera.setPosition(glm::vec2(playerPos.x, playerPos.y));

	m_level.collectGameplayStateData(m_gameplayState);
	m_level.update(m_b2World.get(), deltaTime);
	m_particleSystem.update(deltaTime);

	m_b2World->Step(1.0f / 60.0f, 8, 3);
	m_camera.update(m_game->getWindowDimentions());

	if (m_game->getInputManager().isKeyDown(SDLK_p))
	{
		m_camera.setScale(m_camera.getScale() - 0.001f);
	}

	if (m_game->getInputManager().isKeyDown(SDLK_o))
	{
		m_camera.setScale(m_camera.getScale() + 0.001f);
	}

	if (m_game->getInputManager().isKeyPressed(SDLK_t))
	{
		m_level.removeBossGate(m_b2World.get(), "skeleton_boss_entrance");
		m_level.removeBossGate(m_b2World.get(), "skeleton_boss_outer_exit");
	}

	if (m_game->getInputManager().isKeyPressed(SDLK_ESCAPE) && m_level.getPlayer().getUIState() == UIState::NONE)
	{
		m_menuDisplay = !m_menuDisplay;
	}
	m_menuDisplay ? m_gameplayState = GameplayState::MENU : m_gameplayState = GameplayState::PLAY;

	m_game->getAudioEngine().SetChannel3dPosition(m_ambientChannelId, { playerPos.x / PPM, playerPos.y / PPM, -2.0f });

	auto player = m_level.getPlayer();
	if (player.getUIState() != UIState::NONE || player.getRestState() != RestState::NONE || m_menuDisplay)
	{
		SDL_ShowCursor(SDL_ENABLE);
		m_drawCrosshair = false;
	}
	else
	{
		SDL_ShowCursor(SDL_DISABLE);
		m_drawCrosshair = true;
	}
}

void GameplayScene::draw()
{
	m_framebuffer.bind();
	Engine::Renderer2D::RenderScene("staticBG");
	Engine::Renderer2D::RenderScene("staticBGTrees");

	Engine::Renderer2D::BeginScene("dynamic");
	m_level.drawDynamic(m_particleSystem);
	m_particleSystem.draw(Engine::Renderer2D::GetCurrentBatch());
	Engine::Renderer2D::EndScene("dynamic");

	Engine::Renderer2D::RenderScene("tmxLevel");

	//Engine::Renderer2D::RenderScene("skeleton_boss_entrance");
	//Engine::Renderer2D::RenderScene("skeleton_boss_outer_exit");
	//Engine::Renderer2D::RenderScene("skeleton_boss_exit");

	Engine::Renderer2D::RenderScene("staticLevelObjects");
	Engine::Renderer2D::RenderScene("dynamic");


	Engine::Renderer2D::RenderScene("staticFG");
	Engine::Renderer2D::RenderScene("staticFGNear");

	m_framebuffer.unbind();
	
	float scale = m_camera.getScale();

	Engine::Renderer2D::BeginScene("framebuffer");
	Engine::Renderer2D::DrawTexturedQuad(m_framebuffer.getTexture(), 
		{ m_camera.getPosition().x - (m_screenDims.x / scale / 2.0f), m_camera.getPosition().y - (m_screenDims.y / scale / 2.0f), m_screenDims.x / scale, m_screenDims.y / scale }, 
		{ 0.0f, 0.0f, 1.0f, -1.0f });
	Engine::Renderer2D::EndScene("framebuffer");
	Engine::Renderer2D::RenderScene("framebuffer");

	Engine::Renderer2D::BeginScene("hud", Engine::GlyphSortType::BACK_TO_FRONT);
	drawUI();

	//glm::vec2 dims = m_game->getWindowDimentions();
	//Engine::Renderer2D::DrawTexturedQuad("crate_512x512.png", { m_camera.getPosition() - glm::ivec2(dims / 2.0f), dims }, { 0.0f, 0.0f, 1.0f, 1.0f }, 3.0f);


	Engine::Renderer2D::EndScene("hud");
	Engine::Renderer2D::RenderScene("hud");

	/*Engine::Renderer2D::BeginScene("text");
	drawFonts();
	Engine::Renderer2D::EndScene("text");
	Engine::Renderer2D::RenderScene("text");*/

	//Engine::Renderer2D::BeginScene("debugText");
	//Engine::Renderer2D::DrawText(m_statStream.str(), {m_camera.getPosition() + 
	//	glm::ivec2((-m_game->getWindowDimentions().x / 2) + 16, (m_game->getWindowDimentions().y / 2) - 32)}, 16);
	//Engine::Renderer2D::EndScene("debugText");
	//Engine::Renderer2D::RenderScene("debugText", Engine::ShaderType::SHADER_TEXT);

//#define DEBUG_LINES
#ifdef DEBUG_LINES

	m_lineRenderer.begin();

	auto skeletons = m_level.getSkeletons();

	for (auto& skeleton : skeletons)
	{
		m_lineRenderer.drawLines(skeleton.getPath(),{255, 0, 255, 255});

		//printf("====================================\n");
		//for (auto& point : path)
		//{
		//	printf("-> ( %f, %f ) ", point.x, point.y);
		//}
		//printf("\n====================================\n");
	}

	m_lineRenderer.end();
	m_lineRenderer.setLineSmoothing(true);
	m_lineRenderer.setLineWidth(2.0f);
	m_lineRenderer.render(m_camera);

#endif // DEBUG
}


void GameplayScene::handleInput(double deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		m_game->onSDLEvent(event);
	}
}

void GameplayScene::drawUI()
{
	auto& player = m_level.getPlayer();
	
	drawBonfireMenu(player);
	drawLevelUpMenu(player);
	drawInventoryMenu(player);
	drawTravelMenu(player);

	drawMainMenu(player);

	drawHUD(player);
	drawDebugWindow(player);

	if (m_drawCrosshair)
	{
		drawCrosshair();
	}
}

void GameplayScene::drawBonfireMenu(Player& player)
{
	if (player.isAtBonfire() && player.getRestState() == RestState::SITTING)
	{
		player.setUIState(UIState::UI);

		glm::vec2 pos = m_game->getWindowDimentions() / glm::vec2(2);

		ImGui::SetNextWindowPos(ImVec2(pos.x - 600, pos.y - 300), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(450, 350));

		ImGui::Begin("bonfire mnenu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);


		if (ImGui::Button("Level up", ImVec2(420, 40)))
		{
			player.setRestState(RestState::LEVELUP);
			m_playerData = player.getPlayerData();
			m_initialPlayerData = m_playerData;
		}

		if (ImGui::Button("Travel", ImVec2(420, 40)))
		{
			player.setRestState(RestState::TRAVEL);
		}

		if (ImGui::Button("Inventory", ImVec2(420, 40)))
		{
			player.setRestState(RestState::INVENTORY);
		}

		if (ImGui::Button("Leave", ImVec2(420, 40)))
		{
			m_level.getPlayer().setRestState(RestState::NONE);
			player.leaveBonfire();
			player.setUIState(UIState::NONE);
		}

		ImGui::End();

	}
	
	if (!player.isAtBonfire() && player.getRestState() == RestState::NONE)
	{
		m_level.getPlayer().setRestState(RestState::NONE);
		player.setUIState(UIState::NONE);
	}
}

void GameplayScene::drawLevelUpMenu(Player& player)
{
	if (player.isAtBonfire() && player.getRestState() == RestState::LEVELUP)
	{
		glm::vec2 dims = m_game->getWindowDimentions() - glm::vec2(30);
		ImVec2 thumbnailDims = { 32, 32 };

		float ratio = dims.x / 1920;

		ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(dims.x, dims.y));

		// create a window and append into it
		ImGui::Begin("levelup menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::Columns(3, "jd", false);
		ImGui::SetColumnWidth(0, 700 * ratio);
		ImGui::SetColumnWidth(1, 720 * ratio);
		ImGui::SetColumnWidth(2, 470 * ratio);

		//first child
		ImGui::Dummy({ 300, 100 * ratio });
		ImGui::BeginChild(1, { 700 * ratio, 300 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/ui/levelup_menu_icon_level.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.lvl). c_str(), "Level");

		ImGui::Dummy({ 700, 50 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.experience).c_str(), "Experience");

		uint32_t reqSouls = 0;

		for (uint32_t i = 0; i < m_playerData.lvl; i++)
		{
			reqSouls += getLevelUpCost(m_playerData.lvl);
		}

		reqSouls /= 10;

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(reqSouls).c_str(), "Required souls");

		ImGui::EndChild();

		// second child
		ImGui::Dummy({ 700, 30 * ratio });
		ImGui::BeginChild(2, { 700 * ratio, 550 * ratio });

		ImGui::SetNextItemWidth(700 * ratio);
		ImGui::Text("Attributes");
		ImGui::Dummy({ 700 * ratio, 30 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_initialPlayerData.vigor).c_str(), "Vigor");
		ImGui::SameLine();
		ImGui::Text(" => ");
		ImGui::SameLine();
		ImGui::PushID(1);
		if (ImGui::Button("<", ImVec2(35, 35)))
		{
			if (m_playerData.vigor - 1 >= m_initialPlayerData.vigor && m_playerData.vigor <= 99)
			{
				m_playerData.lvl--;
				m_playerData.vigor--;
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_playerData.vigor).c_str());
		ImGui::SameLine();
		ImGui::PushID(2);
		if (ImGui::Button(">", ImVec2(35, 35)) && m_playerData.experience > getLevelUpCost(m_playerData.lvl + 1))
		{
			if (m_playerData.vigor + 1 >= m_initialPlayerData.vigor && m_playerData.vigor < 99)
			{
				m_playerData.lvl++;
				m_playerData.vigor++;
			}
			
		}
		ImGui::PopID();
		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_initialPlayerData.endurance).c_str(), "Endurance");
		ImGui::SameLine();
		ImGui::Text(" => ");
		ImGui::SameLine();
		ImGui::PushID(3);
		if (ImGui::Button("<", ImVec2(35, 35)))
		{
			if (m_playerData.endurance - 1 >= m_initialPlayerData.endurance && m_playerData.endurance <= 99)
			{
				m_playerData.lvl--;
				m_playerData.endurance--;
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_playerData.endurance).c_str());
		ImGui::SameLine();
		ImGui::PushID(4);
		if (ImGui::Button(">", ImVec2(35, 35)) && m_playerData.experience > getLevelUpCost(m_playerData.lvl + 1))
		{
			if (m_playerData.endurance + 1 >= m_initialPlayerData.endurance && m_playerData.endurance < 99)
			{
				m_playerData.lvl++;
				m_playerData.endurance++;
			}
		}
		ImGui::PopID();
		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_initialPlayerData.strength).c_str(), "Stength");
		ImGui::SameLine();
		ImGui::Text(" => ");
		ImGui::SameLine();
		ImGui::PushID(5);
		if (ImGui::Button("<", ImVec2(35, 35)))
		{
			if (m_playerData.strength - 1 >= m_initialPlayerData.strength && m_playerData.strength <= 99)
			{
				m_playerData.lvl--;
				m_playerData.strength--;
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_playerData.strength).c_str());
		ImGui::SameLine();
		ImGui::PushID(6);
		if (ImGui::Button(">", ImVec2(35, 35)) && m_playerData.experience > getLevelUpCost(m_playerData.lvl + 1))
		{
			if (m_playerData.strength + 1 >= m_initialPlayerData.strength && m_playerData.strength < 99)
			{
				m_playerData.lvl++;
				m_playerData.strength++;
			}
		}
		ImGui::PopID();
		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_initialPlayerData.dexterity).c_str(), "Dexterity");
		ImGui::SameLine();
		ImGui::Text(" => ");
		ImGui::SameLine();
		ImGui::PushID(7);
		if (ImGui::Button("<", ImVec2(35, 35)))
		{
			if (m_playerData.dexterity - 1 >= m_initialPlayerData.dexterity && m_playerData.dexterity <= 99)
			{
				m_playerData.lvl--;
				m_playerData.dexterity--;
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_playerData.dexterity).c_str());
		ImGui::SameLine();
		ImGui::PushID(8);
		if (ImGui::Button(">", ImVec2(35, 35)) && m_playerData.experience > getLevelUpCost(m_playerData.lvl + 1))
		{
			if (m_playerData.dexterity + 1 >= m_initialPlayerData.dexterity && m_playerData.dexterity < 99)
			{
				m_playerData.lvl++;
				m_playerData.dexterity++;
			}
		}
		ImGui::PopID();
		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_initialPlayerData.luck).c_str(), "Luck");
		ImGui::SameLine();
		ImGui::Text(" => ");
		ImGui::SameLine();
		ImGui::PushID(9);
		if (ImGui::Button("<", ImVec2(35, 35)))
		{
			if (m_playerData.luck - 1 >= m_initialPlayerData.luck && m_playerData.luck <= 99)
			{
				m_playerData.lvl--;
				m_playerData.luck--;
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_playerData.luck).c_str());
		ImGui::SameLine();
		ImGui::PushID(10);
		if (ImGui::Button(">", ImVec2(35, 35)) && m_playerData.experience > getLevelUpCost(m_playerData.lvl + 1))
		{
			if (m_playerData.luck + 1 >= m_initialPlayerData.luck && m_playerData.luck < 99)
			{
				m_playerData.lvl++;
				m_playerData.luck++;
			}
		}
		ImGui::PopID();
		ImGui::Dummy({ 300, 50 * ratio });
		if (ImGui::Button("Confirm", { 670 * ratio, 40 }))
		{
			if (m_playerData.experience >= reqSouls && m_playerData.lvl > m_initialPlayerData.lvl)
			{
				printf("Confirming spent levels\n");
				PlayerData data;
				data = m_playerData;
				data.experience -= reqSouls;
				player.setPlayerData(data);

				m_playerData = player.getPlayerData();
				m_initialPlayerData = player.getPlayerData();
				glm::vec2 pos = player.getPosition() / glm::vec2(PPM);
				m_game->getAudioEngine().playSound("assets/audio/levelup.mp3", {pos.x, pos.y, 0});
			}
			
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild(3, { 700 * ratio, 400 * ratio });
		ImGui::Dummy({ 300, 100 * ratio });

		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::Text("Base Power");
		ImGui::Dummy({ 570 * ratio, 30 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.health).c_str(), "HP");

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText("100", "FP");

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.stamina).c_str(), "Stamina");

		ImGui::EndChild();

		ImGui::BeginChild(4, { 700 * ratio, 250 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.equipLoad).c_str(), "Equip Load");

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText("12", "Poise");

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText(std::to_string(m_playerData.itemDiscovery).c_str(), "Item Discovery");

		ImGui::EndChild();

		ImGui::BeginChild(5, { 700 * ratio, 250 * ratio });

		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::Text("Attack Power");
		ImGui::Dummy({ 570 * ratio, 30 * ratio });

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText("5", "Weapon 1");

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, thumbnailDims);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300 * ratio);
		ImGui::LabelText("12", "Weapon 2");

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::Dummy(ImVec2(360 * ratio, 0));
		ImGui::SameLine();
		if (ImGui::Button("X", { 64 * ratio, 64 * ratio }))
		{
			player.setRestState(RestState::SITTING);
		}

		ImGui::End();

	}
}

void GameplayScene::drawInventoryMenu(Player& player)
{
	if (/*player.isAtBonfire() &&*/ player.getRestState() == RestState::INVENTORY)
	{
		glm::vec2 dims = m_game->getWindowDimentions() - glm::vec2(30);
		float ratio = dims.x / 1920;

		float padding = 15.0f;
		float imageSize = 100 * ratio;
		float buttonSize = 30 * ratio;
	
		ImGui::SetNextWindowPos({ 15, 15 }, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ dims.x, dims.y }, ImGuiCond_Always);
		ImGui::Begin("inventory menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::Columns(3, "inventory columns", false);
		ImGui::SetColumnWidth(0, dims.x * 0.4);
		ImGui::SetColumnWidth(1, dims.x * 0.54);
		ImGui::SetColumnWidth(2, dims.x * 0.06);

		ImGui::BeginChild(1, { (dims.x * 0.4f) - (padding * 2.0f), dims.y - (padding * 2.0f)}, false);

		auto items = player.getItems();

		if (items.has_value())
		{
			// loop through all player items
			for (uint32_t i = 0; i < items.value().size(); i++)
			{
				ImTextureID itemImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/" + items.value()[i].getImage()).id;
				ImTextureID useButtonImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/stone.png").id;

				ImGui::Image(itemImage, { imageSize - (padding * 2.0f), imageSize - (padding * 2.0f) });
				ImGui::SameLine();
				ImGui::Text(items.value()[i].getName().c_str()); ImGui::SameLine();

				ImGui::PushID(i + 1);
				ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
				ImGui::PopID();
				ImGui::PushID(i + 2);
				ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
				ImGui::PopID();
				ImGui::PushID(i + 3);
				ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID(i + 4);
				if (ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }))
				{
					m_detailItem = i;
				}
				ImGui::PopID();

				ImGui::Text("Uzycie: %s", items.value()[i].getFor().c_str());

				ImGui::Dummy({ 1, 20 });
			}
		}
		else
		{
			ImGui::Text("No items yet...");
		}

		ImGui::EndChild();
		ImGui::NextColumn();

		ImGui::BeginChild(2, { (dims.x * 0.54f) - (padding * 2.0f), imageSize * 4.0f }, false);

		if (items.has_value())
		{
			ImTextureID itemImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/" + items.value()[m_detailItem].getImage()).id;

			ImGui::Columns(2, "inner columns right panel", false);

			ImGui::SetColumnWidth(0, dims.x * 0.25f - padding * 2.0f);
			ImGui::SetColumnWidth(1, dims.x * 0.35f - padding * 2.0f);

			ImGui::Image(itemImage, { imageSize * 4.0f - (padding * 2.0f), imageSize * 4.0f - (padding * 2.0f) });

			ImGui::NextColumn();
			ImGui::Text(items.value()[m_detailItem].getName().c_str());
			ImGui::Text("Uzycie: %s", items.value()[m_detailItem].getFor().c_str());
		}

		ImGui::EndChild();

		ImGui::BeginChild(3, { (dims.x * 0.54f) - (padding * 2.0f), (dims.y - (padding * 2.0f)) - (imageSize * 4.0f) - 10 });

		if (items.has_value())
		{
			ImGui::TextWrapped(items.value()[m_detailItem].getDescription().c_str());
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		if (ImGui::Button("X", { 64 * ratio, 64 * ratio }))
		{
			if (player.isAtBonfire())
			{
				player.setRestState(RestState::SITTING);
				printf("is at bonfire\n");
			}
			else
			{
				player.setRestState(RestState::NONE);
			}
		}
		
		ImGui::End();

	}
}

void GameplayScene::drawTravelMenu(Player& player)
{
	if (player.isAtBonfire() && player.getRestState() == RestState::TRAVEL)
	{
		glm::vec2 dims = m_game->getWindowDimentions() - glm::vec2(30);
		float ratio = dims.x / 1920.0f;

		ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(dims.x, dims.y));

		ImGui::Begin("bonfire menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::BeginChild(1, { dims.x - (120 * ratio), 110 }, false);

		auto bonfires = m_level.getBonfires();
		auto discoveredAreas = m_level.getDiscoveredAreas();

		for (uint32_t i = 0; i < discoveredAreas.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::ImageButton((void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/watermelon.png").id, { 75, 75 }))
			{
				m_regionSelected = discoveredAreas[i];
			}
			ImGui::PopID();
			ImGui::SameLine();
		}

		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild(2, { 150 * ratio, 110}, false);

		if (ImGui::Button("X", { 64 * ratio, 64 * ratio }))
		{
			player.setRestState(RestState::SITTING);
		}

		ImGui::EndChild();

		ImGui::BeginChild(3, { dims.x, dims.y - 200 }, false);

		ImGui::Columns(2, "columns", false);
		ImGui::SetColumnWidth(0, dims.x / 3.0f);
		ImGui::SetColumnWidth(1, dims.x / 1.5f);

		ImGui::Text(m_regionSelected.c_str());
		ImGui::Dummy({ 0, 20 });

		for (uint32_t i = 0; i < bonfires.size(); i++)
		{

			if (bonfires[i].wasDiscovered() && bonfires[i].region() == m_regionSelected)
			{
				if (ImGui::Button(bonfires[i].getName().c_str(), { 600 * ratio, 40 }))
				{
					m_selectedBonfire = i;
				}
			}
		}

		if (ImGui::Button("tp", { 600 * ratio, 40 }))
		{
			player.setPosition(bonfires[m_selectedBonfire].getPosition());
			player.setLastBonfireRestedAtPosition(bonfires[m_selectedBonfire].getPosition());
			player.setRestState(RestState::NONE);
		}

		ImGui::NextColumn();

		ImGui::Image((void*)(intptr_t)Engine::ResourceManager::getTexture(bonfires[m_selectedBonfire].getThumbnail()).id, { 960 * ratio, 540 * ratio });
		ImGui::Text(bonfires[m_selectedBonfire].getDescription().c_str());

		ImGui::EndChild();

		ImGui::End();

	}
}

void GameplayScene::drawMainMenu(Player& player)
{
	if (m_gameplayState == GameplayState::MENU)
	{
		glm::vec2 dims = m_game->getWindowDimentions() - glm::vec2(30);
		ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(dims.x, dims.y));
		ImGui::Begin("main menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		if (ImGui::Button("Resume", ImVec2(420, 40)))
		{
			m_menuDisplay = false;
		}

		if (ImGui::Button("Inventory", ImVec2(420, 40)))
		{
			player.setRestState(RestState::INVENTORY);
		}
		
		if (ImGui::Button("Leave", ImVec2(420, 40)))
		{
			m_currentState = Engine::SceneState::EXIT_APPLICATION;
		}

		ImGui::End();
	}
}

void GameplayScene::drawHUD(Player& player)
{
	glm::vec2 camPos = m_camera.getPosition();

	glm::vec2 topRightCorner = camPos + glm::vec2(m_screenDims.x / 2, m_screenDims.y / 2);

	if (m_gameplayState == GameplayState::PLAY)
	{

		float maxHealthPosible = player.getMaxPossibleHealth();
		float maxStaminaPosible = player.getMaxPossibleStamina();
		//printf("stamina max: %f\n", maxStaminaPosible);

		glm::vec2 expCounterPos = { camPos.x + (m_screenDims.x / 2) - 300, camPos.y - (m_screenDims.y / 2) + 50 };
		glm::vec2 healthBarPos = { camPos.x - (m_screenDims.x / 2) + 50, camPos.y + (m_screenDims.y / 2) - 66 };

		Engine::Renderer2D::DrawTexturedQuad("ui/label_background_2.png", { expCounterPos.x, expCounterPos.y, 240, 48 });
		Engine::Renderer2D::DrawText(std::to_string(player.getPlayerData().experience), expCounterPos + glm::vec2(220, 8), 24, Engine::Justification::RIGHT);

		//Engine::Renderer2D::DrawTexturedQuad("ui/melon_type_background.png", { healthBarPos.x - 128, healthBarPos.y - 100, 128, 128 });

		// health bar
		Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_background.png", 
			{ healthBarPos.x, healthBarPos.y, 1100 * (static_cast<float>(player.getPlayerData().maxHealth) / maxHealthPosible), 16 },
			{0.0f, 0.0f, static_cast<float>(player.getPlayerData().maxHealth) / maxHealthPosible, 1.0f});

		float x = static_cast<float>(player.getPlayerData().health) / static_cast<float>(player.getPlayerData().maxHealth);
		if (x < 0.0f) x = 0.0f;

		Engine::Renderer2D::DrawTexturedQuad("ui/health_bar.png", 
			{ healthBarPos.x, healthBarPos.y, player.getPlayerData().maxHealth * (1100.0f / maxHealthPosible) * (static_cast<float>(player.getPlayerData().health) / static_cast<float>(player.getPlayerData().maxHealth)), 16 },
			{ 0.0f, 0.0f, x, 1.0f });

		Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_outline.png", { healthBarPos.x, healthBarPos.y - 7, 1100 * (static_cast<float>(player.getPlayerData().maxHealth) / maxHealthPosible), 30 },
			{ 0.0f, 0.0f, static_cast<float>(player.getPlayerData().maxHealth) / maxHealthPosible, 1.0f });

		// stamina bar

		Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_background.png",
			{ healthBarPos.x, healthBarPos.y - 30, 1100 * (static_cast<float>(player.getPlayerData().maxStamina) / maxStaminaPosible), 16 },
			{ 0.0f, 0.0f, static_cast<float>(player.getPlayerData().maxStamina) / maxStaminaPosible, 1.0f });

		float xStamina = static_cast<float>(player.getPlayerData().stamina) / static_cast<float>(player.getPlayerData().maxStamina);
		if (xStamina < 0.0f) xStamina = 0.0f;

		Engine::Renderer2D::DrawTexturedQuad("ui/stamina_bar.png",
			{ healthBarPos.x, healthBarPos.y - 30, player.getPlayerData().maxStamina * (1100.0f / maxStaminaPosible) * (static_cast<float>(player.getPlayerData().stamina) / static_cast<float>(player.getPlayerData().maxStamina)), 16 },
			{ 0.0f, 0.0f, xStamina, 1.0f });

		Engine::Renderer2D::DrawTexturedQuad("ui/stamina_bar_outline.png", { healthBarPos.x, healthBarPos.y - 37, 1100 * (static_cast<float>(player.getPlayerData().maxStamina) / maxStaminaPosible), 30 },
			{ 0.0f, 0.0f, static_cast<float>(player.getPlayerData().maxStamina) / maxStaminaPosible, 1.0f });
	}

	if (SaveManager::Saving() || SaveManager::Loading())
	{
		// play animation

		m_loadingIconAngle += 0.1;
		Engine::Renderer2D::DrawTexturedQuad("ui/loading_icon.png", { topRightCorner.x - 128, topRightCorner.y - 128 , 64, 64 }, m_loadingIconAngle);
	}
}

void GameplayScene::drawDebugWindow(Player& player)
{

	ImGui::Begin("Debug Data", NULL);

	if (ImGui::CollapsingHeader("Player"))
	{
		ImGui::Indent();
		ImGui::Text("Position: x: %f, y: %f", player.getPosition().x, player.getPosition().y);
		ImGui::Text("Position in meters: x: %f, y: %f", player.getPosition().x / PPM, player.getPosition().y / PPM);
		ImGui::Text("Health: %d", player.getPlayerData().health);
		ImGui::Text("Max health: %d", player.getPlayerData().maxHealth);
		ImGui::Unindent();
	}
	ImGui::Text("Fps: %d", m_game->getStats().fps);

	ImGui::End();
}

void GameplayScene::drawCrosshair()
{
	glm::vec2 mouseCoords = m_camera.convertScreenToWorld(m_game->getInputManager().getMouseCoords());
	Engine::ColorRGBA8 color = { 255, 255, 255, 255 };

	Engine::Renderer2D::DrawColoredQuad({ mouseCoords.x - 2, mouseCoords.y + 3, 4, 10}, color);
	Engine::Renderer2D::DrawColoredQuad({ mouseCoords.x - 2, mouseCoords.y - 13, 4, 10}, color);
	Engine::Renderer2D::DrawColoredQuad({ mouseCoords.x - 13, mouseCoords.y -2, 10, 4}, color);
	Engine::Renderer2D::DrawColoredQuad({ mouseCoords.x + 3, mouseCoords.y - 2, 10, 4}, color);
}

uint32_t GameplayScene::getLevelUpCost(uint32_t level)
{
	if (level == 1) return 1;
	if (level == 2) return 2;
	return level * 10 + getLevelUpCost(level - 1);
}

void GameplayScene::setupStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
 
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 2);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;
 
	style->Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.1f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.23f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.22f, 0.21f, 0.28f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}

void GameplayScene::AStarTest()
{
	/*std::vector<std::vector<int>> grid = {
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	};

	printf("tukej\n");
	auto path = Engine::PathFinding::aStarSearch(grid, { 0, 0 }, { 35, 29 });
	printf("tukej2\n");

	for (uint32_t i = 0; i < path.size(); i++)
	{
		printf("-> ( %d, %d ) ", (int)path[i].x, (int)path[i].y);
	}*/

}


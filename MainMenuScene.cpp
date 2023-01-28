#include "MainMenuScene.h"

#include "Indices.h"
#include "../Engine/ResourceManager.h"

#include <imgui.h>

#include <fstream>

MainMenuScene::MainMenuScene()
{

}

MainMenuScene::~MainMenuScene()
{

}

int MainMenuScene::getNextScreenIndex() const
{
	return SCENE_INDEX_GAMEPLAY_SCENE;
}

int MainMenuScene::getPreviousScreenIndex() const
{
	return SCENE_INDEX_NO_SCENE;
}

void MainMenuScene::build()
{

}

void MainMenuScene::destroy()
{

}

void MainMenuScene::onEntry()
{
	preloadTextures();

	setupStyle();

	glm::vec2 dims = m_game->getWindowDimentions();

	m_selectedResolution = std::to_string((uint32_t)dims.x) + "x" + std::to_string((uint32_t)dims.y);
}

void MainMenuScene::onExit()
{
	m_game->getAudioEngine().StopAllChannels();
	m_game->getAudioEngine().ResetPositionAllChannels();
}

void MainMenuScene::update(double deltaTime)
{
	handleInput(deltaTime);
}

void MainMenuScene::draw()
{
	drawMainMenu();
	drawSettingsMenu();
}

void MainMenuScene::handleInput(double deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		m_game->onSDLEvent(event);
	}
}

void MainMenuScene::drawMainMenu()
{
	if (m_currentMenu == Menu::MAIN)
	{
		glm::vec2 dims = m_game->getWindowDimentions();
		float ratio = dims.x / 1920;

		glm::vec2 size = glm::vec2(530.0f, 168.0f);

		ImGui::SetNextWindowPos(ImVec2((dims.x / 2.0f) - size.x / 2.0f, (dims.y / 2) - size.y / 2), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y));

		ImGui::Begin("main menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		if (ImGui::Button("Play", { size.x - 30.0f, 40 }))
		{
			m_currentState = Engine::SceneState::CHANGE_NEXT;
		}

		if (ImGui::Button("Settings", { size.x - 30.0f, 40 }))
		{
			m_currentMenu = Menu::SETTINGS;
		}

		if (ImGui::Button("Exit to Windows", { size.x - 30.0f, 40 }))
		{
			m_currentState = Engine::SceneState::EXIT_APPLICATION;
		}

		ImGui::End();
	}
}

void MainMenuScene::drawSettingsMenu()
{
	if (m_currentMenu == Menu::SETTINGS)
	{
		glm::vec2 dims = m_game->getWindowDimentions();
		float ratio = dims.x / 1920;
		float buttonSize = 300.0f;

		int32_t volume = 40;

		glm::vec2 size = glm::vec2(glm::clamp(dims.x, 1120.0f, 1820.0f), (dims.y - 100.0f) * ratio);

		ImGui::SetNextWindowPos(ImVec2((dims.x / 2.0f) - size.x / 2.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y));

		ImGui::Begin("settings menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::Dummy({ size.x - 100.0f, 0 }); ImGui::SameLine();
		if (ImGui::Button("X", { 64, 64 }))
		{
			m_currentMenu = Menu::MAIN;
		}

		ImGui::Dummy({ dims.x / 10.0f, 0 }); ImGui::SameLine();

		if (ImGui::Button("General", { buttonSize * ratio, 40 }))
		{
			m_currentSetting = Setting::GENERAL;
		}
		ImGui::SameLine();

		if (ImGui::Button("Audio", { buttonSize * ratio, 40 }))
		{
			m_currentSetting = Setting::AUDIO;
		}
		ImGui::SameLine();

		if (ImGui::Button("Video", { buttonSize * ratio, 40 }))
		{
			m_currentSetting = Setting::VIDEO;
		}
		ImGui::SameLine();


		ImGui::Dummy({ 0, 100 * ratio });
		ImGui::Columns(3, "setting columns");
		ImGui::SetColumnWidth(0, size.x / 4.0f);
		ImGui::SetColumnWidth(1, size.x / 2.0f);
		ImGui::SetColumnWidth(2, size.x / 4.0f);

		ImGui::NextColumn();

		drawGeneralSettings(size);
		drawAudioSettings(size);
		drawVideoSettings(size);

		ImGui::End();
	}
}

void MainMenuScene::drawGeneralSettings(const glm::vec2& size)
{
	if (m_currentSetting == Setting::GENERAL)
	{

	}
}

void MainMenuScene::drawAudioSettings(const glm::vec2& size)
{
	if (m_currentSetting == Setting::AUDIO)
	{
		float width = size.x / 2.0f;

		ImGui::BeginChild(1, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("Master Volume:");

		ImGui::NextColumn();

		ImGui::Text("2317 / 2137");

		ImGui::EndChild();

		ImGui::BeginChild(2, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("Dialogue Volume:");

		ImGui::NextColumn();

		ImGui::Text("2317 / 2137");

		ImGui::EndChild();

		ImGui::BeginChild(3, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("SFX Volume:");

		ImGui::NextColumn();

		ImGui::Text("2317 / 2137");

		ImGui::EndChild();

		ImGui::BeginChild(4, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("Ambient Volume:");

		ImGui::NextColumn();

		ImGui::Text("2317 / 2137");

		ImGui::EndChild();
	}
}

void MainMenuScene::drawVideoSettings(const glm::vec2& size)
{
	if (m_currentSetting == Setting::VIDEO)
	{
		float width = size.x / 2.0f;

		ImGui::BeginChild(1, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("Window resolution:");

		ImGui::NextColumn();

		std::vector<std::string> items = { "1920x1080", "1600x900", "1280x768" };

		if (ImGui::BeginCombo("", m_selectedResolution.c_str(), ImGuiComboFlags_NoArrowButton))
		{
			for (uint32_t i = 0; i < items.size(); i++)
			{
				bool selected = (m_selectedResolution == items[i]);

				if (ImGui::Selectable(items[i].c_str(), selected))
				{
					m_selectedResolution = items[i];
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
					m_madeChanges = true;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::EndChild();

		ImGui::BeginChild(2, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);

		ImGui::Text("Window Mode:");

		ImGui::NextColumn();

		std::vector<std::string> windowModes = { "borderless", "fullscreen", "windowed"};

		if (ImGui::BeginCombo("", m_selectedWindowMode.c_str(), ImGuiComboFlags_NoArrowButton))
		{
			for (uint32_t i = 0; i < windowModes.size(); i++)
			{
				bool selected = (m_selectedWindowMode == windowModes[i]);

				if (ImGui::Selectable(windowModes[i].c_str(), selected))
				{
					m_selectedWindowMode = windowModes[i];
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
					m_madeChanges = true;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::EndChild();

		ImGui::BeginChild(3, { size.x / 2, 50 });

		ImGui::Columns(2, "inner setings columns 2", true);
		ImGui::SetColumnWidth(0, width / 3.0f);
		ImGui::SetColumnWidth(1, width * 0.66f);


		ImGui::NextColumn();

		if (ImGui::Button("Apply", { width / 2.5f , 40.0f }) && m_madeChanges)
		{
			applySettings();
		}

		ImGui::EndChild();
	}
}

void MainMenuScene::applySettings()
{
	printf("applying changes\n");
	if (m_selectedResolution == "1920x1080")
	{
		m_game->getWindow().setWindowDimentions({ 1920, 1080 });
		SDL_SetWindowPosition(m_game->getWindow().get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	if (m_selectedResolution == "1600x900")
	{
		m_game->getWindow().setWindowDimentions({ 1600, 900 });
	}

	if (m_selectedResolution == "1280x768")
	{
		m_game->getWindow().setWindowDimentions({ 1280, 768 });
	}

	if (m_selectedWindowMode == "borderless")
	{
		SDL_SetWindowFullscreen(m_game->getWindow().get(), (SDL_bool)false);
		SDL_SetWindowBordered(m_game->getWindow().get(), (SDL_bool)false);
	}

	if (m_selectedWindowMode == "windowed")
	{
		SDL_SetWindowFullscreen(m_game->getWindow().get(), (SDL_bool)false);
		SDL_SetWindowBordered(m_game->getWindow().get(), (SDL_bool)true);
		SDL_SetWindowPosition(m_game->getWindow().get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	if (m_selectedWindowMode == "fullscreen")
	{
		SDL_SetWindowFullscreen(m_game->getWindow().get(), (SDL_bool)true);
	}

	m_madeChanges = false;
}

void MainMenuScene::setupStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
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
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	//style->Colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.1f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.23f, 0.0f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.22f, 0.21f, 0.28f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.10f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
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
	style->Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.1f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.30f);
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

void MainMenuScene::preloadTextures()
{
	std::ifstream in("assets/textures/textures.preload");

	if (in.fail())
		printf("failed to load preload textures\n");

	std::string file, filter;
	while (!in.eof())
	{
		in >> file >> filter;
		bool filtering = false;
		if (filter == "true") filtering = true;
		if (filter == "false") filtering = false;
		Engine::ResourceManager::loadTexture(file, filtering);
	}
	in.close();
}
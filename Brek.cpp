#include "Brek.h"

#include "../Engine/Renderer2D.h"

#include <imgui.h>


Brek::Brek()
{

}

Brek::~Brek()
{

}

void Brek::init(b2World* world, const glm::vec2& position)
{
	m_dimentions = { 100, 100 };

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x / PPM, position.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(m_dimentions.x / PPM / 2, m_dimentions.y / PPM / 2);

	b2PolygonShape sensorShape;
	sensorShape.SetAsBox(m_dimentions.x / PPM, m_dimentions.y / PPM / 2);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.filter.groupIndex = -1;
	m_body->CreateFixture(&fixtureDef);

	b2FixtureDef sensorDef;
	sensorDef.shape = &sensorShape;
	sensorDef.isSensor = true;
	m_body->CreateFixture(&sensorDef);

	m_sprite.init("assets/textures/atlases/niebieski_chop.png", { position, m_dimentions }, { 3, 5 }, 14, 0.0f, false);

	m_entityData = std::make_shared<EntityData>();
	m_entityData.get()->type = "npc";
	m_body->SetUserData(m_entityData.get());

}

void Brek::draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims)
{
	Engine::Renderer2D::DrawAnimatedQuad(m_sprite);

	if (m_dialogueState == DialogueState::DISPLAY)
	{
		Engine::Renderer2D::DrawTexturedQuad("ui/dialogue_background_fade.png",
			{ camera.getPosition().x - screenDims.x / 2, camera.getPosition().y - screenDims.y / 2, screenDims.x, 256 });

		Engine::Renderer2D::DrawText("Lato-Regular.ttf", getDialogue(m_currentDialogueName).getLine(m_dialogueLineNumber), camera.getPosition() - glm::ivec2(0, (screenDims.y / 2) - 50), 24, Engine::Justification::MIDDLE);
	}

	if (m_dialogueState == DialogueState::DISPLAY_UI)
	{
		drawUI(screenDims);
	}
}

ReturnType Brek::update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager)
{
	if (m_dialogueState == DialogueState::DISPLAY)
	{
		float padding = 800.0f;
		if (m_player->getPosition().x  > (m_body->GetPosition().x * PPM) + padding || m_player->getPosition().x < (m_body->GetPosition().x * PPM) - padding)
		{
			m_dialogueState = DialogueState::NONE;
			resetDialogue(audioEngine);
		}

		if (!getDialogue(m_currentDialogueName).wasAudioPlayed(m_dialogueLineNumber))
		{
			getDialogue(m_currentDialogueName).playAudio(audioEngine, m_dialogueLineNumber, { m_body->GetPosition().x, m_body->GetPosition().y, 0 }, 0.0f);
			getDialogue(m_currentDialogueName).setAudioPlayed(m_dialogueLineNumber);
		}

		if (m_dialogueTimer.ElapsedMillis() >= getDialogue(m_currentDialogueName).getDisplayTime(m_dialogueLineNumber) * 1000.0f)
		{
			m_dialogueLineNumber++;
			m_dialogueTimer.Reset();
		}

		if (m_dialogueLineNumber > getDialogue(m_currentDialogueName).getLineNumber() - 1)
		{
			m_dialogueState = DialogueState::DISPLAY_UI;
			resetDialogue(audioEngine);
		}
	}

	if (m_dialogueState == DialogueState::DISPLAY_UI)
	{
		m_player->setRestState(RestState::NPC_DIALOGUE);
	}

	glm::vec2 pos = glm::vec2((m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2);

	m_sprite.update(deltaTime);
	m_sprite.setPosition(pos);
	m_sprite.playAnimation(1.0f / 5.0f);

	if (m_entityData.get()->contact)
	{
		if (m_entityData.get()->with == "player")
		{
			return ReturnType::COLLISION_PLAYER;
		}
	}

	return ReturnType::NONE;
}

void Brek::playDialogue()
{
	m_dialogueTimer.Reset();
	m_dialogueState = DialogueState::DISPLAY;
	m_currentDialogueName = "default";
}

void Brek::resetDialogue(Engine::AudioEngine& audioEngine)
{
	m_dialogueLineNumber = 0;
	m_dialogueTimer.Reset();
	
	getDialogue(m_currentDialogueName).skipAudio(m_dialogueLineNumber, audioEngine);
	getDialogue(m_currentDialogueName).reset();
}

void Brek::drawUI(const glm::vec2& screenDims)
{
	if (m_dialogueState == DialogueState::DISPLAY_UI)
	{
		drawNPCMenu(screenDims);
	}
}

void Brek::drawNPCMenu(const glm::vec2& screenDims)
{
	glm::vec2 pos = screenDims / glm::vec2(2);

	ImGui::SetNextWindowPos(ImVec2(pos.x - 600, pos.y - 300), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(450, 350));

	ImGui::Begin("jebac disa", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);


	if (m_removedObstacle)
	{
		if (ImGui::Button("Talk", ImVec2(420, 40)))
		{
			m_dialogueTimer.Reset();
			m_dialogueState = DialogueState::DISPLAY;
			m_currentDialogueName = "removed_gate";
			m_player->setRestState(RestState::NONE);
		}
	}

	if (!m_removedObstacle)
	{
		if (ImGui::Button("Talk", ImVec2(420, 40)))
		{
			m_dialogueTimer.Reset();
			m_dialogueState = DialogueState::DISPLAY;
			m_currentDialogueName = "default";
			m_player->setRestState(RestState::NONE);
		}
	}

	if (m_canRemoveObstacle && !m_removedObstacle)
	{
		if (ImGui::Button("Remove obstacle", ImVec2(420, 40)))
		{
			m_dialogueTimer.Reset();
			m_actionReturnType = NPCActionReturnType::REMOVE_BOSS_GATE;
			m_dialogueState = DialogueState::DISPLAY;
			m_currentDialogueName = "removing_gate";
			m_player->setRestState(RestState::NONE);
			m_removedObstacle = true;
		}
	}

	if (m_numExplosives < 2 && m_canGiveExplosives && m_player->getItemsByName("Dynamit").has_value())
	{
		if (ImGui::Button("Give explosives", ImVec2(420, 40)))
		{
			if (m_player->getItemsByName("Dynamit").value().size() >= 2)
			{
				m_numExplosives += 2 - m_numExplosives;
				m_player->removeItemsFromInventoryByName("Dynamit", 2);
			}
			else
			{
				m_numExplosives++;
			}

			m_dialogueTimer.Reset();
			m_dialogueState = DialogueState::DISPLAY;
			m_player->setRestState(RestState::NONE);

			if (m_numExplosives == 1)
			{
				m_currentDialogueName = "explosives_given";
			}

			if (m_numExplosives == 2)
			{
				m_currentDialogueName = "enough_explosives";
				m_canGiveExplosives = false;
				m_canRemoveObstacle = true;
			}
		}
	}

	if (ImGui::Button("Leave", ImVec2(420, 40)))
	{
		m_dialogueTimer.Reset();
		m_dialogueState = DialogueState::NONE;
		m_player->setRestState(RestState::NONE);
	}

	ImGui::End();
}
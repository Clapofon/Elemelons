#pragma once

#include "NPC.h"


class Brek : public NPC
{
public:

	Brek();
	virtual ~Brek();

	void init(b2World* world, const glm::vec2& position);
	virtual void onInit() {}
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) override;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) override;
	NPCActionReturnType getActionType() { return m_actionReturnType; }

	void setActionType(NPCActionReturnType type) { m_actionReturnType = type; }
	void setIfCanRemoveObstacle(bool val) { m_canRemoveObstacle = val; }

	void playDialogue();

private:
	void resetDialogue(Engine::AudioEngine& audioEngine);

	void drawUI(const glm::vec2& screenDims);
	void drawNPCMenu(const glm::vec2& screenDims);

	DialogueState m_dialogueState = DialogueState::NONE;
	uint32_t m_dialogueLineNumber = 0;
	Engine::Timer m_dialogueTimer;
	std::string m_currentDialogueName = "default";

	bool m_removedObstacle = false;
	bool m_canRemoveObstacle = false;
	bool m_canGiveExplosives = true;

	int16_t m_numExplosives = 0;
};


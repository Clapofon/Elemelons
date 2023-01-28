#pragma once

#include "Enemy.h"
#include "Player.h"

class Spider : public Enemy
{
public:

	Spider();
	virtual ~Spider();

	void init(b2World* world, const glm::vec2& position);
	void onInit() override;

	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) override;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) override;

	void collectPlayerData(Player* player) { m_player = player; }

private:

	void move(double deltaTime, const glm::vec2& aggroPadding, const glm::vec2& attackPadding, float velocity, float jumpPower);
	void attack();
	void takeDamage();

	void drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims);

	float m_maxSpeed = 3.0f;

	AnimationState m_animationState = AnimationState::IDLE;

	Player* m_player = nullptr;
	glm::vec2 m_playerPosition = { 0, 0 };

	bool m_canAttack = false;
	bool m_isPlayerClose = false;
	bool m_isPlayerInAgrroRange = false;


};


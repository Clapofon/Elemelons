#pragma once

#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "EntityData.h"
#include "CapsuleCollider.h"

#include "../Engine/Timer.h"
#include "../Engine/Renderer2D.h"

#include <vector>

enum class AIState
{
	PATROLING,
	INVESTIGATING,
	CHASING
};


class Skeleton : public Enemy
{
public:
	Skeleton();
	virtual ~Skeleton();

	void init(b2World* world, const glm::vec2& position);
	virtual void onInit() override;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) override;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) override;

	void collectPlayerData(Player* player);
	void collectGridData(const std::vector<std::vector<int>>& grid);
	void collectProjectileData(std::vector<Projectile>* projectiles);

	std::vector<glm::vec2> getPath() { return m_path; }
	//glm::vec2 getInitialPosition() { return m_initialPosition; }

	//int16_t getHealth() { return m_health; }
	//int16_t getMaxHealth() { return m_maxHealth; }
	//uint32_t getExperienceYield() { return m_experienceYield; }

	//void setPosition(glm::vec2 pos) { m_capsuleCollider.getBody()->SetTransform(b2Vec2(pos.x / PPM, pos.y / PPM), 0.0f); }
	//void setHealth(int16_t health) { m_health = health; }
	//void setInitialPosition(const glm::vec2& position) { m_initialPosition = position; }

	bool isPlayerClose() { return m_isPlayerClose; }
	bool isPlayerInAgrroRange() { return m_isPlayerInAgrroRange; }

protected:
	void move(double deltaTime, const glm::vec2& aggroPadding, const glm::vec2& attackPadding, float velocity, float jumpPower);
	void attack();
	void takeDamage();
	void shootProjectile(b2World* world, const glm::vec2& size, const glm::vec2& colliderSize, const glm::vec2& power, glm::vec2 offset);

	void drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims);

	bool canJump();

	Player* m_player;

	glm::vec2 m_playerPosition = { 0, 0 };
	float m_maxSpeed = 0.5f;
	bool m_canAttack = true;
	bool m_isPlayerClose = false;
	bool m_isPlayerInAgrroRange = false;
	Engine::Timer m_timer;
	Engine::Timer m_audioTimer;
	bool m_canPlayAudio = true;

	AnimationState m_animationState = AnimationState::IDLE;
	AIState m_aiState = AIState::PATROLING;

	Engine::AnimatedSprite m_attackSprite;
	Engine::AnimatedSprite m_deathSprite;
	Engine::AnimatedSprite m_idleSprite;

	std::vector<glm::vec2> m_path;
	std::vector<std::vector<int>> m_grid;
	std::vector<Projectile>* m_projectiles;

	Engine::Timer m_jumpTimer;
	bool m_canJump = false;

	//glm::vec2 m_initialPosition = { 0, 0 };

};


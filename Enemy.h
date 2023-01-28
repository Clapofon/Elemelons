#pragma once

#include "Actor.h"

class Enemy : public Actor
{
public:
	Enemy();
	virtual ~Enemy();

	virtual void onInit() = 0;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) = 0;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) = 0;

	void setPosition(glm::vec2 pos) { m_capsuleCollider.getBody()->SetTransform(b2Vec2(pos.x / PPM, pos.y / PPM), 0.0f); }
	void setHealth(int16_t health) { m_health = health; }
	void setInitialPosition(const glm::vec2& position) { m_initialPosition = position; }

	int16_t getHealth() { return m_health; }
	int16_t getMaxHealth() { return m_maxHealth; }
	uint32_t getExperienceYield() { return m_experienceYield; }
	glm::vec2 getInitialPosition() { return m_initialPosition; }

	glm::vec2 getPosition() { return { m_capsuleCollider.getBody()->GetPosition().x * PPM, m_capsuleCollider.getBody()->GetPosition().y * PPM }; }
	glm::vec2 getDimentions() { return m_dimentions; }

	b2Body* getBody() { return m_capsuleCollider.getBody(); }
	b2Fixture* getFixture(uint32_t index) { return m_capsuleCollider.getFixture(index); }

protected:

	CapsuleCollider m_capsuleCollider;

	uint32_t m_experienceYield = 0;

	std::shared_ptr<EntityData> m_data;

	int16_t m_health = 100;
	int16_t m_maxHealth = 100;

	glm::vec2 m_initialPosition = { 0, 0 };
};


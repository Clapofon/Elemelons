#pragma once

#include "Skeleton.h"

#include "EntityData.h"

class SkeletonBoss : public Skeleton
{
public:
	SkeletonBoss();
	~SkeletonBoss();

	void init(b2World* world, const glm::vec2 pos);
	virtual void onInit() override;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) override;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) override;

	std::string getName() { return m_name; }

	void setName(const std::string& name) { m_name = name; }
	void setHealth(uint16_t health) { m_health = health; }

	void showHealthBar() { m_drawHealthBar = true; }
	void hideHealthBar() { m_drawHealthBar = false; }

private:

	void drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims);

	bool m_drawHealthBar = false;

	std::string m_name;
};


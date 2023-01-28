#pragma once

#include "../Engine/AnimatedSprite.h"
#include "../Engine/ParticleSystem2D.h"
#include "../Engine/Constants.h"
#include "../Engine/AudioEngine.h"
#include "../Engine/InputManager.h"

#include "EntityData.h"
#include "CapsuleCollider.h"

#include <glm.hpp>
#include <Box2D/Box2D.h>

class Actor
{
public:
	Actor() {}
	virtual ~Actor() {}

	void init(b2World* world, const glm::vec2& position) 
	{
		

		onInit();
	}

	virtual void onInit() = 0;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) = 0;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) = 0;

protected:
	
	Engine::AnimatedSprite m_sprite;

	glm::vec2 m_dimentions = { 100, 100 };
};


#pragma once

#include "../Engine/Object.h"

#include "EntityData.h"

class Projectile : public Engine::Object
{
public:
	Projectile();
	virtual ~Projectile();

	virtual void init(b2World* world, const glm::vec2& pos, const glm::vec2& dimentions, const glm::vec2& colliderDimentions, const b2Vec2& initialVelocity, 
		float angle, const std::string& type, uint32_t collisionGroupIndex);
	virtual void draw();

	ReturnType update(double deltaTime);


private:

	float m_angle = 0.0f;

	std::shared_ptr<EntityData> m_data;
};


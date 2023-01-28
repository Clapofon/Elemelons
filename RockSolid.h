/*#pragma once

#include "EntityData.h"

#include "../Engine/Constants.h"
#include "../Engine/Renderer2D.h"

#include <Box2D/Box2D.h>

class RockSolid
{
public:

	RockSolid();
	~RockSolid();

	void init(b2World* world, const glm::vec2& pos, const b2Vec2& initialVelocity);
	void draw();

	ReturnType update(double deltaTime);

	glm::vec4 getDestRect() { return glm::vec4(m_body->GetPosition().x * PPM, m_body->GetPosition().y * PPM, m_dimentions.x, m_dimentions.y); }
	float getAngle() { return m_body->GetAngle(); }

	b2Body* getBody() { return m_body; }

private:
	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;
	glm::vec2 m_dimentions = glm::vec2(100, 100);

	std::shared_ptr<EntityData> m_data;
};*/


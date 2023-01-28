#pragma once

#include <Box2D/Box2D.h>
#include <glm.hpp>

#include "EntityData.h"

#include "../Engine/Constants.h"

class Rope
{
public:

	Rope();
	~Rope();

	void init(b2World* world, const glm::vec2& position, float length);
	void draw();
	ReturnType update();

	void applyForce(const glm::vec2& force);

	void attachPlayer() { m_playerAttached = true; }
	void dettachPlayer() { m_playerAttached = false; }

	void setActive(bool val) { m_bodies[0]->SetActive(val); m_bodies[1]->SetActive(val); }
	void setAwake(bool val) { m_bodies[0]->SetAwake(val); m_bodies[1]->SetAwake(val); }

	glm::vec2 getEndPosition() { return glm::vec2(m_bodies[1]->GetPosition().x * PPM, m_bodies[1]->GetPosition().y * PPM); };
	b2Vec2 getVelocity() { return m_bodies[1]->GetLinearVelocity(); }

private:

	std::shared_ptr<EntityData> m_data;

	b2Body* m_bodies[2];

	bool m_playerAttached = false;
};


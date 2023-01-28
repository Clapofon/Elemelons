#include "Rope.h"

#include "../Engine/Renderer2D.h"

Rope::Rope()
{

}

Rope::~Rope()
{

}

void Rope::init(b2World* world, const glm::vec2& position, float length)
{
	b2BodyDef bodyDefA;
	bodyDefA.type = b2_staticBody;
	bodyDefA.position.Set(position.x / PPM, position.y / PPM);
	m_bodies[0] = world->CreateBody(&bodyDefA);

	b2BodyDef bodyDefB;
	bodyDefB.type = b2_dynamicBody;
	bodyDefB.position.Set(position.x / PPM, position.y / PPM);

	b2CircleShape circleShape;
	circleShape.m_radius = 0.5;

	b2FixtureDef fixtureDefB;
	fixtureDefB.isSensor = true;
	fixtureDefB.shape = &circleShape;

	m_bodies[1] = world->CreateBody(&bodyDefB);
	m_bodies[1]->CreateFixture(&fixtureDefB);


	// Now create the main swinging joint.
	b2RopeJointDef jointDef;
	jointDef.bodyA = m_bodies[0];
	jointDef.bodyB = m_bodies[1];
	jointDef.localAnchorA = m_bodies[0]->GetPosition();
	jointDef.localAnchorB = m_bodies[1]->GetPosition();
	jointDef.maxLength = length;
	jointDef.collideConnected = true;
	jointDef.localAnchorA.Set(0.0f, 0.0f);
	jointDef.localAnchorB.Set(0.0f, 0.0f);
	world->CreateJoint(&jointDef);

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "rope";
	m_bodies[1]->SetUserData(m_data.get());
}

void Rope::draw()
{
	Engine::Renderer2D::DrawTexturedQuad("tree1.png", { (m_bodies[1]->GetPosition().x * PPM) - 50, (m_bodies[1]->GetPosition().y * PPM) - 50, 100, 100 });
	Engine::Renderer2D::DrawTexturedQuad("tree1.png", { (m_bodies[0]->GetPosition().x * PPM) - 50, (m_bodies[0]->GetPosition().y * PPM) - 50, 100, 100 });
}

ReturnType Rope::update()
{
	if (m_playerAttached)
	{
		m_bodies[1]->SetLinearVelocity({ m_bodies[1]->GetLinearVelocity().x * 0.999f, m_bodies[1]->GetLinearVelocity().y });
	}
	else
	{
		m_bodies[1]->SetLinearVelocity({ m_bodies[1]->GetLinearVelocity().x * 0.9f, m_bodies[1]->GetLinearVelocity().y });
	}

	if (m_data.get()->contact)
	{
		if (m_data.get()->with == "player")
		{
			return ReturnType::COLLISION_PLAYER;
		}
	}

	return ReturnType::NONE;
}

void Rope::applyForce(const glm::vec2& force)
{
	m_bodies[1]->ApplyLinearImpulse({ force.x, force.y }, m_bodies[1]->GetWorldPoint({ 0, 0 }), true);
}
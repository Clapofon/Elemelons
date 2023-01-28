#include "Crate.h"

#include "../Engine/ResourceManager.h"

Crate::Crate()
{

}

Crate::~Crate()
{

}

void Crate::init(b2World* world, const glm::vec2& pos, const glm::vec2& dimentions, bool staticBody)
{
	m_dimentions = dimentions;
	b2BodyDef bodyDef;
	(staticBody) ? bodyDef.type = b2_staticBody : bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
	bodyDef.fixedRotation = false;
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(m_dimentions.x / 2 / PPM, m_dimentions.y / 2 / PPM);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 1.3f;
	m_body->CreateFixture(&fixtureDef);
}

void Crate::draw()
{
	Engine::Renderer2D::DrawTexturedQuad("crate_512x512.png",
		{ (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2, m_dimentions.x, m_dimentions.y }, m_body->GetAngle());
}

/*#include "RockSolid.h"

RockSolid::RockSolid()
{

}

RockSolid::~RockSolid()
{

}

void RockSolid::init(b2World* world, const glm::vec2& pos, const b2Vec2& initialVelocity)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = m_dimentions.x / 2.0f;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 30.0f;
	fixtureDef.friction = 2.0f;
	m_fixture = m_body->CreateFixture(&fixtureDef);

	m_body->ApplyLinearImpulse(initialVelocity, m_body->GetWorldPoint(b2Vec2(0.0f, 0.0f)), true);

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "rocksolid";
	m_body->SetUserData(m_data.get());
}

void RockSolid::draw()
{
	Engine::Renderer2D::DrawTexturedQuad("obstacles/rock.png", { (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2, m_dimentions }, 
		m_body->GetAngle(), { 0.0f, 0.0f, 1.0f, 1.0f }, { 255, 255, 255, 255 }, false);
}

ReturnType RockSolid::update(double deltaTime)
{

}
*/
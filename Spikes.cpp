#include "Spikes.h"
#include "../Engine/ResourceManager.h"

Spikes::Spikes()
{

}

Spikes::~Spikes()
{

}

void Spikes::init(b2World* world, const glm::vec2& pos)
{
	m_dimentions = { 128, 128 };

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
	bodyDef.fixedRotation = false;
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(m_dimentions.x / 2 / PPM, m_dimentions.y / 2 / PPM);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	m_body->CreateFixture(&fixtureDef);

	m_data = new EntityData;
	m_data->type = "spikes";
	m_body->SetUserData(m_data);
}

void Spikes::draw()
{
	Engine::Renderer2D::DrawTexturedQuad("spikes.png",
		{ (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2, m_dimentions.x, m_dimentions.y },
		{ 0.0f, 0.0f, 1.0f, 1.0f }, 1.0f, {255, 255, 255, 255}, false);
}

bool Spikes::update(double deltaTime)
{
	return m_data->contact;
}
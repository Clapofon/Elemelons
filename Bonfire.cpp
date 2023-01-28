#include "Bonfire.h"

#include "../Engine/Renderer2D.h"
#include "../Engine/Constants.h"

Bonfire::Bonfire()
{

}

Bonfire::~Bonfire()
{

}

void Bonfire::init(b2World* world, const glm::vec2& position, const glm::vec2& dimentions, const std::string& textureAtlas, const std::string& name, 
	const std::string& thumbnail, const std::string& desc, const std::string& region)
{
	m_dimentions = dimentions;
	m_sprite.init("assets/textures/atlases/" + textureAtlas, { position, dimentions }, { 1, 6 }, 5, 1.0, false);
	m_name = name;
	m_thumbnail = "assets/textures/ui/" + thumbnail;
	m_description = desc;
	m_region = region;

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(position.x / PPM, position.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(m_dimentions.x / 2 / PPM, m_dimentions.y / 2 / PPM);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	m_body->CreateFixture(&fixtureDef);


	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "bonfire";
	m_body->SetUserData(m_data.get());

}

void Bonfire::draw()
{
	Engine::Renderer2D::DrawAnimatedQuad(m_sprite);
}

bool Bonfire::update(double deltaTime)
{
	m_sprite.update(deltaTime);
	m_sprite.setPosition({ (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2 });
	m_sprite.playAnimation(1.0f / 10.0f);

	return m_data.get()->contact;
}

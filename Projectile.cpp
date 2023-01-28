#include "Projectile.h"

Projectile::Projectile()
{

}

Projectile::~Projectile()
{
	
}

void Projectile::init(b2World* world, const glm::vec2& pos, const glm::vec2& dimentions, const glm::vec2& colliderDimentions, const b2Vec2& initialVelocity, 
	float angle, const std::string& type, uint32_t collisionGroupIndex)
{
	m_dimentions = dimentions;

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(colliderDimentions.x / 2 / PPM, colliderDimentions.y / 2 / PPM);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	//fixtureDef.friction = 3.0f;
	//fixtureDef.filter.groupIndex = collisionGroupIndex;
	fixtureDef.isSensor = true;
	m_fixture = m_body->CreateFixture(&fixtureDef);

	m_body->ApplyLinearImpulse(initialVelocity, m_body->GetWorldPoint(b2Vec2(0.0f, 0.0f)), true);
	m_angle = angle;

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = type;
	m_body->SetUserData(m_data.get());
}

void Projectile::draw()
{
	Engine::Renderer2D::DrawTexturedQuad("projectiles/arrow.png", { (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2, m_dimentions }, m_angle,
		{ 0.0f, 0.0f, 1.0f, 1.0f }, 1.0f, {255, 255, 255, 255}, false);
}

ReturnType Projectile::update(double deltaTime)
{
	if (m_data.get()->contact)
	{
		if (m_data.get()->with == "skeleton")
		{
			return ReturnType::COLLISION_SKELETON;
		}

		if (m_data.get()->with == "spider")
		{
			return ReturnType::COLLISION_SPIDER;
		}

		if (m_data.get()->with == "player")
		{
			return ReturnType::COLLISION_PLAYER;
		}

		if (m_data.get()->with == "skeletonBoss")
		{
			return ReturnType::COLLISION_BOSS_SKELETON;
		}
	}

	return ReturnType::NONE;
}
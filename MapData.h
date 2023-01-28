#pragma once

#include <string>

#include <glm.hpp>
#include <Box2D/Box2D.h>

#include "EntityData.h"
#include "../Engine/AnimatedSprite.h"

struct Sprite
{
	std::string texture = "";
	glm::vec2 dimentions = { 0, 0 };
	glm::vec2 position = { 0, 0 };
};

struct DeadSkeleton
{
	Sprite sprite;
	glm::vec2 initialPosition;
};

struct BossRoomGate
{
	std::string name = "";
	b2Body* body = nullptr;
	std::vector<b2Vec2> points;

	b2Body* getBody() { return body; }
};

class SoulsYielded
{
public:
	SoulsYielded()
	{}

	~SoulsYielded()
	{}

	void init(b2World* world, const glm::vec2& position, uint64_t amount)
	{
		m_position = position;
		m_amount = amount;

		glm::vec2 dims = { 250, 500 };

		m_sprite.init("assets/textures/atlases/fireAtlas.png", { m_position.x - (dims.x / 2.0f), position.y - (dims.y / 2.0f), dims}, { 4, 8 }, 31, 1.0, true);

		m_data = std::make_shared<EntityData>();
		m_data.get()->type = "souls";


		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set(m_position.x / PPM, m_position.y / PPM);
		m_body = world->CreateBody(&bodyDef);

		b2CircleShape circleShape;
		circleShape.m_radius =  200 / PPM;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &circleShape;
		fixtureDef.isSensor = true;
		m_body->CreateFixture(&fixtureDef);

		m_body->SetUserData(m_data.get());
	}

	ReturnType update(double deltaTime)
	{
		m_sprite.update(deltaTime);
		m_sprite.setPosition(m_position - glm::vec2(50));
		m_sprite.playAnimation(1.0f / 2.0f);

		if (m_data.get()->contact)
		{
			if (m_data.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}

		return ReturnType::NONE;
	}

	void draw()
	{
		Engine::Renderer2D::DrawAnimatedQuad(m_sprite);
	}

	b2Body* getBody() { return m_body; }
	uint64_t getAmount() { return m_amount; }
	glm::vec2 getPosition() { return m_position; }

private:
	Engine::AnimatedSprite m_sprite;
	uint64_t m_amount = 0;
	glm::vec2 m_position = { 0, 0 };

	std::shared_ptr<EntityData> m_data;

	b2Body* m_body = nullptr;
};

class Trigger
{
public:
	Trigger() {}
	~Trigger() {}

	void init(b2World* world, const glm::vec2& pos, const glm::vec2& dims, const std::string& whatFor)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set((pos.x + (dims.x / 2)) / PPM, (pos.y - (dims.y / 2)) / PPM);
		m_body = world->CreateBody(&bodyDef);

		b2PolygonShape quadShape;
		quadShape.SetAsBox(dims.x / 2.0f / PPM, dims.y / 2.0f / PPM);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &quadShape;
		fixtureDef.isSensor = true;
		m_body->CreateFixture(&fixtureDef);

		m_data = std::make_shared<EntityData>();

		m_data.get()->type = "trigger";
		m_body->SetUserData(m_data.get());

		m_for = whatFor;
	}

	void execute()
	{
		printf("walked into trigger\n");
	}

	ReturnType update()
	{
		if (m_data.get()->contact)
		{
			if (m_data.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}

		return ReturnType::NONE;
	}

	b2Body* getBody() { return m_body; }

	std::string what() { return m_for; }

private:
	b2Body* m_body = nullptr;

	std::shared_ptr<EntityData> m_data;
	std::string m_for = "default";
};
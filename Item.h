#pragma once

#include <string>
#include <memory>

#include <glm.hpp>
#include <Box2D/Box2D.h>

#include "EntityData.h"
#include "../Engine/Constants.h"

class Item
{
public:
	Item() {}
	~Item() {}

	void init(b2World* world, const std::string& id, const std::string& name, const std::string& purpose, const std::string& image, const std::string& description, const glm::vec4& destRect, bool isInInventory = false)
	{
		m_id = id;
		m_name = name;
		m_for = purpose;
		m_image = image;
		m_description = description;
		m_destRect = destRect;
		m_inPlayerInventory = isInInventory;

		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set(destRect.x / PPM, destRect.y / PPM);
		m_body = world->CreateBody(&bodyDef);

		b2CircleShape circleShape;
		circleShape.m_radius = destRect.z / PPM;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &circleShape;
		fixtureDef.isSensor = true;
		m_body->CreateFixture(&fixtureDef);

		m_entityData = std::make_shared<EntityData>();
		m_entityData.get()->type = "item";
		m_body->SetUserData(m_entityData.get());
	}

	ReturnType update()
	{
		if (m_entityData.get()->contact)
		{
			
			if (m_entityData.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}

		return ReturnType::NONE;
	}

	std::string getFor() { return m_for; }
	std::string getId() { return m_id; }
	std::string getName() { return m_name; }
	std::string getImage() { return m_image; }
	std::string getDescription() { return m_description; }
	glm::vec4 getDestRect() { return m_destRect; }
	bool inPlayerInventory() { return m_inPlayerInventory; }

	void setIsInPlayerInventory(bool val) { m_inPlayerInventory = val; }

private:
	std::string m_id;
	std::string m_name;
	std::string m_for;
	std::string m_image;
	std::string m_description;

	glm::vec4 m_destRect = { 0, 0, 0, 0 };

	bool m_inPlayerInventory = false;

	std::shared_ptr<EntityData> m_entityData;

	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;


};


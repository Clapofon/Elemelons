#pragma once

#include <glm.hpp>
#include <Box2D/Box2D.h>

#include "../Engine/AnimatedSprite.h"
#include "../Engine/Object.h"

#include "EntityData.h"

class Bonfire : public Engine::Object
{
public:
	Bonfire();
	~Bonfire();

	void init(b2World* world, const glm::vec2& position, const glm::vec2& dimentions, const std::string& textureAtlas, const std::string& name, 
		const std::string& thumbnail, const std::string& desc, const std::string& region);
	void draw();
	bool update(double deltaTime);

	glm::vec2 getPosition() { return glm::vec2(m_body->GetPosition().x * PPM, m_body->GetPosition().y * PPM); }
	std::string getName() { return m_name; }
	std::string getThumbnail() { return m_thumbnail; } //lightweight babyyyy!
	std::string getDescription() { return m_description; } 
	bool wasDiscovered() { return m_discovered; } 
	std::string region() { return m_region; }

	void setDiscovered() { m_discovered = true; } 

private:
	Engine::AnimatedSprite m_sprite;
	std::string m_name = "deafult";
	std::string m_thumbnail = "deafult";
	std::string m_description = "deafult";
	std::string m_region = "default";
	bool m_discovered = false;

	bool m_playerContact = false;

	std::shared_ptr<EntityData> m_data;
};


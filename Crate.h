#pragma once

#include "../Engine/SpriteBatch.h"
#include "../Engine/Object.h"
#include "../Engine/Constants.h"

#include "EntityData.h"

#include <Box2D/Box2D.h>

class Crate : public Engine::Object
{
public:
	Crate();
	~Crate();

	virtual void init(b2World* world, const glm::vec2& pos, const glm::vec2& dimentions,  bool staticBody = false);
	virtual void draw() override;

private:
	//std::shared_ptr<EntityData> m_data;
};


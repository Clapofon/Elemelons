#pragma once

#include "../Engine/Object.h"
#include "../Engine/Constants.h"

#include "EntityData.h"

class Spikes : public Engine::Object
{
public:
	Spikes();
	~Spikes();


	virtual void init(b2World* world, const glm::vec2& pos);
	virtual void draw() override;
	bool update(double deltaTime);

private:

	EntityData* m_data = nullptr;
};


#pragma once

#include "Map.h"

class World
{
public:

	World();
	~World();

	void addMap(const std::string& filename, const glm::vec2& position, float scaling, b2World* world, Engine::IMainGame* parentGame, const std::string& batchName, Engine::Camera* cam);

	void init(b2World* world, Engine::Camera& camera, Engine::IMainGame* game);
	void update(b2World* world, double deltaTime);
	void draw(Engine::ParticleSystem2D& particleSystem);

	Player& getPlayer() { return m_player; }
	std::vector<Bonfire> getBonfires() { return m_maps[0].getBonfires(); }

	void collectGameplayStateData(GameplayState state) { m_gameplayState = state; m_maps[0].collectGameplayStateData(m_gameplayState); }

private:

	void updateAudioEngine(double deltaTime);

	std::vector<Map> m_maps;

	ContactListener m_contactListener;
	GameplayState m_gameplayState = GameplayState::PLAY;

	Player m_player;
	glm::vec2 m_positionCurrentFrame = { 0, 0 }, m_positionLastFrame = { 0, 0 };

	Engine::IMainGame* m_parentGame = nullptr;

	//Map map;
};


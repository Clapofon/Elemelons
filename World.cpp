#include "World.h"

World::World()
{

}

World::~World()
{

}

void World::addMap(const std::string& filename, const glm::vec2& position, float scaling, b2World* world, Engine::IMainGame* parentGame, const std::string& batchName, Engine::Camera* cam)
{
    //Map map;
    //map.init("assets/maps/map1.tmx", { 0, 6064 }, 4.0f, world, parentGame, "tmxLevel", cam);
    //map.drawStatic();

	//m_maps.push_back(map);
}

void World::init(b2World* world, Engine::Camera& camera, Engine::IMainGame* game)
{
   m_player.collectCameraData(&camera);
   world->SetContactListener(&m_contactListener);

   m_parentGame = game;
}

void World::update(b2World* world, double deltaTime)
{
    updateAudioEngine(deltaTime);
    m_maps[0].update(world, deltaTime);
}

void World::draw(Engine::ParticleSystem2D& particleSystem)
{
    m_maps[0].drawDynamic(particleSystem);
}

void World::updateAudioEngine(double deltaTime)
{
    m_positionCurrentFrame = m_player.getPosition() / glm::vec2(PPM);
    glm::vec2 velocity = glm::vec2(m_positionCurrentFrame - m_positionLastFrame) / glm::vec2(deltaTime);
    m_positionLastFrame = m_positionCurrentFrame;

    m_parentGame->getAudioEngine().Set3dListenerAndOrientation({ m_player.getPosition().x / PPM, m_player.getPosition().y / PPM, -2.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f }, { velocity.x, velocity.y, 0.0f });
}
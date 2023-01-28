#include "Map.h"

#include "../Engine/ResourceManager.h"
#include "../Engine/Constants.h"
#include "../Engine/Renderer2D.h"
#include "../Engine/PathFinding.h"
#include "../Engine/StringUtil.h"

#include "EntityData.h"

#include <imgui.h>


Map::Map()
{

}

Map::~Map()
{

}

void Map::onInit(b2World* world, float scaling, float tileHeight)
{
    for (uint32_t i = 0; i < m_skeletons.size(); i++)
    {
        m_skeletons[i].collectPlayerData(&m_player);
        m_skeletons[i].collectGridData(m_pathFindingGrid);
        m_skeletons[i].collectProjectileData(&m_projectiles);
    }

    for (uint32_t i = 0; i < m_spiders.size(); i++)
    {
        m_spiders[i].collectPlayerData(&m_player);
        //m_spiders[i].collectGridData(m_pathFindingGrid);
        //m_spiders[i].collectProjectileData(&m_projectiles);
    }

    for (uint32_t i = 0; i < m_skeletonBosses.size(); i++)
    {
        m_skeletonBosses[i].collectPlayerData(&m_player);
        m_skeletonBosses[i].collectGridData(m_pathFindingGrid);
        m_skeletonBosses[i].collectProjectileData(&m_projectiles);
    }

    m_player.collectProjectileData(&m_playerProjectiles);
    m_player.collectCameraData(m_camera);
    world->SetContactListener(&m_contactListener);

    m_discoveredLevels.push_back("Starting Area");

    //save();

    bool load = true;

    if (load)
    {
        SaveManager::Load("save/save1.xml", &m_loadData, Type::DEFFERED);
        
        if (m_loadData.loaded)
            loadScene(world);
    }
    /*printf("pathFindingGrid size: %d, %d\n", (int)m_pathFindingGrid.size(), (int)m_pathFindingGrid[0].size());

    glm::vec2 playerPos = m_player.getPosition();   
    glm::ivec2 pathFindingPlayerPos = glm::ivec2(playerPos.x / scaling / tileHeight, -((playerPos.y / scaling / tileHeight) - 48));
    printf("pos: %d, %d\n", pathFindingPlayerPos.x, pathFindingPlayerPos.y);

    Engine::Timer timer;
    m_path = Engine::PathFinding::aStarSearch(m_pathFindingGrid, { 7, 7 }, { pathFindingPlayerPos.y, pathFindingPlayerPos.x });
    printf("Timer: %f\n", timer.ElapsedMillis() / 1000);

    for (uint32_t i = 0; i < m_path.size(); i++)
    {
        m_path[i] = {(m_path[i].x * 4 * 32), -((m_path[i].y * 4 * 32) - (48 * 32 * 4))};
    }*/
}

void Map::onPlayerParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    if (group->GetNumObjects() == 1)
    {
        auto player = group->GetObject(0);

        if (!player)
        {
            printf("Player not found in map, spawning at ( 2000, 2000 )\n");
            m_player.init(world, { 2000, 2000 });
        }
        else
        {
            m_player.init(world, mapToWorld({ player->GetX(), player->GetY() }, scaling, tileHeight), PlayerType::WATERMELON);
        }
    }
    else
    {
        printf("Player not found in map, spawning at ( 2000, 2000 )\n");
        m_player.init(world, { 2000, 2000 });
    }
}

void Map::onRectColliderParse(Engine::Tmx::Object* object, b2World* world, float scaling, uint32_t tileHeight)
{
    glm::vec2 dimentions = glm::vec2(object->GetWidth() * scaling, object->GetHeight() * scaling);
    glm::vec2 mapPos = mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight);
    glm::vec2 position = glm::vec2(mapPos.x + dimentions.x / 2.0f, mapPos.y);
    //glm::vec2 position = glm::vec2((object->GetX() * scaling) + dimentions.x / 2.0f, -((object->GetY() * scaling) - (48 * tileHeight * scaling)));

    b2BodyDef colliderDef;
    colliderDef.position.Set(position.x / PPM, position.y / PPM);
    b2Body* colliderBody = world->CreateBody(&colliderDef);

    b2PolygonShape colliderBox;
    colliderBox.SetAsBox(dimentions.x / 2.0f / PPM, dimentions.y / 2.0f / PPM);
    colliderBody->CreateFixture(&colliderBox, 0.0f);

    m_colliders.push_back(colliderBody);
}

void Map::onChainShapeColliderParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    std::vector<b2Vec2> vertices;

    auto& points = group->GetObjects();
    for (auto& point : points)
    {
        glm::vec2 mapPos = mapToWorld({ point->GetX(), point->GetY() }, scaling, tileHeight);
        vertices.push_back(b2Vec2(mapPos.x / PPM, (mapPos.y + 50) / PPM));
    }

    b2Body* chainBody = nullptr;

    b2BodyDef chainDef;
    chainDef.type = b2_staticBody;
    chainBody = world->CreateBody(&chainDef);

    b2ChainShape chainShape;
    chainShape.CreateChain(vertices.data(), vertices.size());
    b2FixtureDef chainFixture;
    chainFixture.shape = &chainShape;
    chainFixture.density = 1.0f;
    chainFixture.friction = 0.3f;
    chainFixture.restitution = 0.0f;
    chainBody->CreateFixture(&chainFixture);

    std::string type = group->GetProperties().GetStringProperty("type");
    if (type == "boss_room_gate_collider")
    {
        std::string name = group->GetProperties().GetStringProperty("name");

        BossRoomGate gate;
        gate.name = name;
        gate.body = chainBody;
        gate.points = vertices;
        m_bossRoomGates.push_back(gate);
    }

    m_colliders.push_back(chainBody);
}

void Map::onObjectDynamicParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        if (object->GetType() == "crate")
        {
            glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
            Crate newCrate;
            glm::vec mapPos = mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight);
            newCrate.init(world, mapPos, dimentions);
            m_dynamicCrates.push_back(newCrate);
        }

        if (object->GetType() == "bonfire")
        {
            glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width") * scaling, object->GetProperties().GetIntProperty("height") * scaling);
            glm::vec2 mapPos = mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight);
            glm::vec2 position = glm::vec2(mapPos.x, mapPos.y + (dimentions.y / 2.0f));
            std::string textureAtlas = object->GetProperties().GetStringProperty("texture");
            std::string soundName = object->GetProperties().GetStringProperty("sound");
            std::string name = object->GetProperties().GetStringProperty("name");
            std::string thumbnail = object->GetProperties().GetStringProperty("thumbnail");
            std::string description = object->GetProperties().GetStringProperty("description");
            std::string region = object->GetProperties().GetStringProperty("region");

            Bonfire bonfire;
            bonfire.init(world, position, dimentions, textureAtlas, name, thumbnail, description, region);
            m_bonfires.push_back(bonfire);

            if (soundName != "")
            {
                m_parentGame->getAudioEngine().LoadSound("assets/audio/" + soundName, true, true, false);
                m_parentGame->getAudioEngine().playSound("assets/audio/" + soundName, { position.x / PPM, position.y / PPM, 0 }, -5.0f);
            }
        }

        if (object->GetType() == "chest")
        {
            std::string closedTexture = object->GetProperties().GetStringProperty("closed_texture");
            std::string openedTexture = object->GetProperties().GetStringProperty("opened_texture");
            std::string spriteFullTexture = object->GetProperties().GetStringProperty("opened_sprite_full");
            std::string openingTexture = object->GetProperties().GetStringProperty("opening_chest");
            std::string inventoryFilename = object->GetProperties().GetStringProperty("inventoryFile");

            glm::vec2 dims = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
            glm::vec2 pos = glm::vec2(mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight));
            glm::vec2 rowsAndCols = glm::vec2(object->GetProperties().GetIntProperty("rows"), object->GetProperties().GetIntProperty("cols"));

            auto items = getChestItems(world, inventoryFilename);

            Chest chest;
            if (items.has_value())
            {
                chest.init(world, pos, dims, "assets/textures/atlases/" + openingTexture, "assets/textures/atlases/" + spriteFullTexture, closedTexture, openedTexture, items.value());
            }
            else
            {
                std::vector<Item> placeholderItems = {};
                chest.init(world, pos, dims, "assets/textures/atlases/" + openingTexture, "assets/textures/atlases/" + spriteFullTexture, closedTexture, openedTexture, placeholderItems);
            }
            
            m_chests.push_back(chest);
        }

        if (object->GetType() == "rope")
        {
            Rope rope;
            rope.init(world, mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight), object->GetProperties().GetFloatProperty("length"));
            m_ropes.push_back(rope);
        }
    }
}

void Map::onObjectStaticParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        if (object->GetType() == "crate")
        {
            glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
            Crate newCrate;
            newCrate.init(world, mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight), dimentions, true);
            m_staticCrates.push_back(newCrate);
        }

        if (object->GetType() == "spike")
        {
            Spikes newSpikes;
            newSpikes.init(world, mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight));
            m_staticSpikes.push_back(newSpikes);
        }

        if (object->GetType() == "sprite")
        {
            std::string texture = object->GetProperties().GetStringProperty("texture");
            glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
            glm::vec2 position = glm::vec2(mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight));
            
            Sprite sprite;
            sprite.texture = texture;
            sprite.dimentions = dimentions;
            sprite.position = position;
            m_sprites.push_back(sprite);
        }

        if (object->GetType() == "cloud")
        {
            std::string texture = object->GetProperties().GetStringProperty("texture");
            glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
            glm::vec2 position = glm::vec2(mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight));

            Sprite sprite;
            sprite.texture = texture;
            sprite.dimentions = dimentions;
            sprite.position = position;
            m_clouds.push_back(sprite);
        }
    }
}

void Map::onVegetationStaticParse(Engine::Tmx::ObjectGroup* group, float scaling)
{
    std::string batchName = group->GetProperties().GetStringProperty("batchName");
    Engine::Renderer2D::BeginScene(batchName);
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        uint32_t tileHeight = m_map.GetTileHeight();
        uint32_t tileWidth = m_map.GetTileWidth();
        std::string texture = object->GetProperties().GetStringProperty("texture");
        glm::vec2 dimentions = glm::vec2(object->GetProperties().GetIntProperty("width"), object->GetProperties().GetIntProperty("height"));
        glm::vec2 position = glm::vec2(mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight));
        float angle = object->GetProperties().GetFloatProperty("angle");
        bool flip = object->GetProperties().GetBoolProperty("flipX");

        if (flip)
        {
            Engine::Renderer2D::DrawTexturedQuad(texture, { position, dimentions }, angle, {0.0f, 0.0f, -1.0f, 1.0f});
        }
        else
        {
            Engine::Renderer2D::DrawTexturedQuad(texture, { position, dimentions }, angle);
        }
    }

    Engine::Renderer2D::EndScene(batchName);
}

void Map::onAudioParse(Engine::Tmx::ObjectGroup* group, float scaling, uint32_t tileHeight)
{
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        if (object->GetType() == "ambient")
        {
            std::string sound = object->GetProperties().GetStringProperty("sound");
            std::string path = "assets/audio/ambient/" + sound;
            glm::vec2 pos = mapToWorld({object->GetX(), object->GetY()}, scaling, tileHeight);

            m_parentGame->getAudioEngine().LoadSound(path, true, true, true);
            m_parentGame->getAudioEngine().playSound(path, { pos.x / PPM, pos.y / PPM, -1.5f }, 5.0f);
        }
    }
}

void Map::onEnemiesParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        if (object->GetType() == "skeleton")
        {
            glm::vec2 position = { mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight) };

            Skeleton skeleton;
            skeleton.init(world, position);
            m_skeletons.push_back(skeleton);
        }

        if (object->GetType() == "spider")
        {
            glm::vec2 position = { mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight) };

            Spider spider;
            spider.init(world, position);
            m_spiders.push_back(spider);
        }
    }
}

void Map::onBossesParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& objects = group->GetObjects();
    for (auto& object : objects)
    {
        if (object->GetType() == "skeleton")
        {
            glm::vec2 position = { mapToWorld({ object->GetX(), object->GetY() }, scaling, tileHeight) };

            SkeletonBoss skeleton;
            skeleton.init(world, position);
            skeleton.setName("Dadziak");
            m_skeletonBosses.push_back(skeleton);
        }
    }
}

void Map::onNPCsParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& npcs = group->GetObjects();
    for (auto& npc : npcs)
    {
        if (npc->GetType() == "Brek")
        {
            Brek brek;
            brek.init(world, { mapToWorld({ npc->GetX(), npc->GetY() }, scaling, tileHeight) });
            brek.collectPlayerData(&m_player);
            brek.loadDialogue(npc->GetProperties().GetStringProperty("dialogueFilename"));
            m_breks.push_back(brek);
        }
    }
}

void Map::onItemsParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& items = group->GetObjects();

    for (uint32_t i = 0; i < items.size(); i++)
    {
        if (items[i]->GetType() == "Explosive")
        {
            std::string id = items[i]->GetProperties().GetStringProperty("id") + std::to_string(i);
            std::string name = items[i]->GetProperties().GetStringProperty("name");
            std::string purpose = items[i]->GetProperties().GetStringProperty("purpose");
            std::string description = items[i]->GetProperties().GetStringProperty("description");
            std::string texture = items[i]->GetProperties().GetStringProperty("texture");

            glm::vec2 pos = glm::vec2(mapToWorld({ items[i]->GetX(), items[i]->GetY() }, scaling, tileHeight));
            glm::vec2 dimentions = glm::vec2(items[i]->GetProperties().GetFloatProperty("width"), items[i]->GetProperties().GetFloatProperty("height"));

            Item item;
            item.init(world, id, name, purpose, texture, description, { pos, dimentions });
            
            m_inventory.addItem(item);
        }
    }
}

void Map::onTriggersParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight)
{
    auto& triggers = group->GetObjects();

    for (uint32_t i = 0; i < triggers.size(); i++)
    {
        if (triggers[i]->GetType() == "OneTime")
        {
            glm::vec2 pos = mapToWorld({ triggers[i]->GetX(), triggers[i]->GetY() }, scaling, tileHeight);
            glm::vec2 dims = { triggers[i]->GetWidth() * scaling, triggers[i]->GetHeight() * scaling };

            Trigger trigger;
            trigger.init(world, pos, dims, triggers[i]->GetName());

            m_oneTimeTriggers.push_back(trigger);
        }
    }
}

void Map::onUpdate(b2World* world, double deltaTime)
{
    if (m_saveTimer.ElapsedMillis() >= 2000.0f)
    {

    }
    
    if (m_saveTimer.ElapsedMillis() >= 1000.0f * 30.0f)
    {
        save();
        m_saveTimer.Reset();
    }

    removeOutOfBoundsObjects(world);

    updateAudioEngine(deltaTime);
    updateDynamicObjects(world, deltaTime);
    updateStaticObjects(world, deltaTime);
    updateEnemies(world, deltaTime);
    updateBosses(world, deltaTime);
    updateNPCs(world, deltaTime);
    updateProjectiles(world, deltaTime);
    updateItems(world, deltaTime);
    updateTriggers(world, deltaTime);
    updateRopes();

    if (m_player.isAttachedToRope())
    {
        uint32_t id = m_player.getRopeIndex();

        if (m_player.canDettach())
        {
            if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
            {
                m_player.dettachFromRope(m_ropes[id].getVelocity());
                m_ropes[id].dettachPlayer();
            }
        }

        m_player.setPosition(m_ropes[id].getEndPosition());

        if (m_parentGame->getInputManager().isKeyDown(SDLK_a) || m_parentGame->getInputManager().isKeyDown(SDLK_LEFT))
        {
            m_ropes[id].applyForce({ -0.2f, 0 });
        }

        if (m_parentGame->getInputManager().isKeyDown(SDLK_d) || m_parentGame->getInputManager().isKeyDown(SDLK_RIGHT))
        {
            m_ropes[id].applyForce({ 0.2f, 0 });
        }
    }

    ReturnType type = m_player.update(world, m_parentGame->getInputManager(), deltaTime, m_gameplayState);
    if (type == ReturnType::DEATH)
    {
        resetEnemiesHealth();
        resetEnemyPositions(world);
        m_playedDyingSound = false;
        m_skeletonBosses[0].hideHealthBar();
        m_player.dettachFromRope({ 0, 0 });
    }

    if (type == ReturnType::DYING && !m_playedDyingSound)
    {
        m_player.dettachFromRope({ 0, 0 });
        m_parentGame->getAudioEngine().playSound("assets/audio/melon_death.wav", { m_player.getPosition().x / PPM, m_player.getPosition().y / PPM, -2.0f });
        m_playedDyingSound = true;

        if (m_soulsYielded.size() > 0)
        {
            world->DestroyBody(m_soulsYielded[0].getBody());
            m_soulsYielded.clear();
        }

        SoulsYielded souls;
        souls.init(world, m_player.getPosition(), m_player.getPlayerData().experience);

        m_soulsYielded.push_back(souls);

        PlayerData data = m_player.getPlayerData();
        data.experience = 0;
        m_player.setPlayerData(data);

        //printf("laying souls\n");
    }
}

void Map::updateAudioEngine(double deltaTime)
{
    m_positionCurrentFrame = m_player.getPosition() / glm::vec2(PPM);
    glm::vec2 velocity = glm::vec2(m_positionCurrentFrame - m_positionLastFrame) / glm::vec2(deltaTime);
    m_positionLastFrame = m_positionCurrentFrame;

    m_parentGame->getAudioEngine().Set3dListenerAndOrientation({ m_player.getPosition().x / PPM, m_player.getPosition().y / PPM, -2.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f }, { velocity.x, velocity.y, 0.0f });
}

void Map::onDrawDynamic(Engine::ParticleSystem2D& particleSystem)
{
    drawDynamicObjects();
    drawEnemies(particleSystem);
    drawBosses(particleSystem);
    drawNPCs(particleSystem);
    drawItems();
    drawRopes();
    drawDeadEnemies();
    m_player.draw(particleSystem);
}

void Map::onDrawStatic()
{
    drawStaticObjects();
}

void Map::drawEnemies(Engine::ParticleSystem2D& particleSystem)
{
    for (uint32_t i = 0; i < m_skeletons.size(); i++)
    {
        m_skeletons[i].draw(particleSystem, *m_camera, m_parentGame->getWindowDimentions());
    }

    for (uint32_t i = 0; i < m_spiders.size(); i++)
    {
        m_spiders[i].draw(particleSystem, *m_camera, m_parentGame->getWindowDimentions());
    }
}

void Map::drawBosses(Engine::ParticleSystem2D& particleSystem)
{
    for (uint32_t i = 0; i < m_skeletonBosses.size(); i++)
    {
        m_skeletonBosses[i].draw(particleSystem, *m_camera, m_parentGame->getWindowDimentions());
    }
}

void Map::drawNPCs(Engine::ParticleSystem2D& particleSystem)
{
    for (auto& brek : m_breks)
    {
        brek.draw(particleSystem, *m_camera, m_parentGame->getWindowDimentions());
    }
}

void Map::drawItems()
{
    auto items = m_inventory.getItems();

    if (!items.has_value())
        return;

    for (auto& item : items.value())
    {
        if (!item.inPlayerInventory())
            Engine::Renderer2D::DrawTexturedQuad(item.getImage(), item.getDestRect());
    }
}

void Map::drawRopes()
{
    for (auto& rope : m_ropes)
    {
        rope.draw();
    }
}

void Map::drawDynamicObjects()
{
    for (uint32_t i = 0; i < m_dynamicCrates.size(); i++)
    {
        m_dynamicCrates[i].draw();
    }

    for (uint32_t i = 0; i < m_bonfires.size(); i++)
    {
        m_bonfires[i].draw();
    }

    for (uint32_t i = 0; i < m_chests.size(); i++)
    {
        m_chests[i].draw();
    }

    for (uint32_t i = 0; i < m_projectiles.size(); i++)
    {
        m_projectiles[i].draw();
    }

    for (uint32_t i = 0; i < m_playerProjectiles.size(); i++)
    {
        m_playerProjectiles[i].draw();
    }

    for (uint32_t i = 0; i < m_soulsYielded.size(); i++)
    {
        m_soulsYielded[i].draw();
    }

    if (m_chests.size() > 0)
        drawChestInventory(m_chests[m_openedChestId]);
}

void Map::drawStaticObjects()
{
    Engine::Renderer2D::BeginScene("staticLevelObjects");

    for (uint32_t i = 0; i < m_staticCrates.size(); i++)
    {
        m_staticCrates[i].draw();
    }

    for (uint32_t i = 0; i < m_staticSpikes.size(); i++)
    {
        m_staticSpikes[i].draw();
    }

    for (uint32_t i = 0; i < m_sprites.size(); i++)
    {
        Engine::Renderer2D::DrawTexturedQuad(m_sprites[i].texture, { m_sprites[i].position, m_sprites[i].dimentions });
    }

    Engine::Renderer2D::EndScene("staticLevelObjects");

    Engine::Renderer2D::BeginScene("staticBG");

    for (int32_t i = -10; i < 10; i++)
    {
        if (i % 2 == 0)
        {
            Engine::Renderer2D::DrawTexturedQuad("sky.png", { i * 1920, -1620, 1920, 2160 }, { 0.0f, 0.0f, -1.0f, 1.0f });
        }
        else
        {
            Engine::Renderer2D::DrawTexturedQuad("sky.png", { i * 1920, -1620, 1920, 2160 });
        }
    }

    for (uint32_t i = 0; i < m_clouds.size(); i++)
    {
        Engine::Renderer2D::DrawTexturedQuad(m_clouds[i].texture, { m_clouds[i].position, m_clouds[i].dimentions });
    }

    Engine::Renderer2D::EndScene("staticBG");
}

void Map::updateDynamicObjects(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_bonfires.size(); i++)
    {
        if (m_bonfires[i].update(deltaTime) && !m_player.isAtBonfire())
        {
            if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
            {
                printf("Used bonfire\n");
                resetEnemyPositions(world);
                m_player.restAtBonfire(m_bonfires[i]);
                resetEnemiesHealth();
                removeProjectiles(world);
                m_bonfires[i].setDiscovered();

                // save game

                save();
            }
        }
    }

    for (uint32_t i = 0; i < m_chests.size(); i++)
    {
        ReturnType type = m_chests[i].update(deltaTime);

        if (type == ReturnType::COLLISION_PLAYER)
        {
            if (m_chests[i].isInventoryOpened())
            {
                m_drawChestInventory = true;
                m_openedChestId = i;
            }

            if (m_chests[i].getState() == ChestState::CLOSED)
            {
                if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
                {
                    m_chests[i].setState(ChestState::OPENING);
                }
            }

            if (m_chests[i].getState() == ChestState::OPENED_FULL || m_chests[i].getState() == ChestState::OPENED_EMPTY)
            {
                if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
                {
                    // open chest inventory

                    m_chests[i].openInventory();
                    m_player.setUIState(UIState::UI);
                    m_player.setRestState(RestState::CHEST_OPENED);
                }
            }
        }
    }

    for (uint32_t i = 0; i < m_soulsYielded.size(); i++)
    {
        ReturnType type = m_soulsYielded[i].update(deltaTime);

        if (type == ReturnType::COLLISION_PLAYER  && m_player.getAnimationState() != PlayerAnimationState::DEAD)
        {
            if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
            {
                // pickup souls

                PlayerData data = m_player.getPlayerData();
                data.experience += m_soulsYielded[i].getAmount();
                m_player.setPlayerData(data);
                world->DestroyBody(m_soulsYielded[i].getBody());
                m_soulsYielded.clear();
            }
        }
    }
}

void Map::updateStaticObjects(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_staticSpikes.size(); i++)
    {
        if (m_staticSpikes[i].update(deltaTime))
        {
            m_player.setHealth(0);
        }
        //else
        //{
        //    i++;
        //}
    }
}

void Map::updateEnemies(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_skeletons.size();)
    {
        ReturnType type = m_skeletons[i].update(deltaTime, world, m_parentGame->getAudioEngine(), m_parentGame->getInputManager());

        if (type == ReturnType::DEATH)
        {
            Sprite sprite;
            sprite.texture = "enemies/skeleton/skeletonDead.png";
            sprite.position = { m_skeletons[i].getPosition().x, m_skeletons[i].getPosition().y };
            sprite.dimentions = { m_skeletons[i].getDimentions().x, m_skeletons[i].getDimentions().y };

            DeadSkeleton deadSkeleton;
            deadSkeleton.sprite = sprite;
            deadSkeleton.initialPosition = m_skeletons[i].getInitialPosition();
            m_deadSkeletons.push_back(deadSkeleton);

            PlayerData data = m_player.getPlayerData();
            data.experience += m_skeletons[i].getExperienceYield();
            m_player.setPlayerData(data);

            m_parentGame->getAudioEngine().playSound("assets/audio/enemies/skeleton/dosc.mp3", { m_skeletons[i].getPosition().x / PPM, m_skeletons[i].getPosition().y / PPM, -1 });

            world->DestroyBody(m_skeletons[i].getBody());
            m_skeletons[i] = m_skeletons.back();
            m_skeletons.pop_back();
        }
        else if (type == ReturnType::COLLISION_PLAYER_PROJECTILE)
        {
            float strength = glm::pow(glm::log(m_player.getPlayerData().strength) * 3.0f, 1);
            float dexterity = glm::pow(glm::log(m_player.getPlayerData().dexterity) * 3.0f, 1);
            //printf("s: %f, d: %f\n", strength, dexterity);

            m_skeletons[i].setHealth(m_skeletons[i].getHealth() - (strength * dexterity));
            i++;
        }
        else
        {
            i++;
        }
    }

    for (uint32_t i = 0; i < m_spiders.size();)
    {
        ReturnType type = m_spiders[i].update(deltaTime, world, m_parentGame->getAudioEngine(), m_parentGame->getInputManager());

        if (type == ReturnType::DEATH)
        {
            Sprite sprite;
            sprite.texture = "enemies/spider/spiderDead.png";
            sprite.position = { m_spiders[i].getPosition().x, m_spiders[i].getPosition().y };
            sprite.dimentions = { m_spiders[i].getDimentions().x, m_spiders[i].getDimentions().y };

            //DeadSpider deadSpider;
            //deadSpider.sprite = sprite;
            //deadSpider.initialPosition = m_spiders[i].getInitialPosition();
            //m_deadSpiders.push_back(deadSpider);

            PlayerData data = m_player.getPlayerData();
            data.experience += m_spiders[i].getExperienceYield();
            m_player.setPlayerData(data);

            world->DestroyBody(m_spiders[i].getBody());
            m_spiders[i] = m_spiders.back();
            m_spiders.pop_back();
        }
        else if (type == ReturnType::COLLISION_PLAYER_PROJECTILE)
        {
            float strength = glm::pow(glm::log(m_player.getPlayerData().strength) * 3.0f, 1);
            float dexterity = glm::pow(glm::log(m_player.getPlayerData().dexterity) * 3.0f, 1);
            //printf("s: %f, d: %f\n", strength, dexterity);

            m_spiders[i].setHealth(m_spiders[i].getHealth() - (strength * dexterity));
            i++;
        }
        else
        {
            i++;
        }
    }
}

void Map::updateBosses(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_skeletonBosses.size(); i++)
    {
        ReturnType type = m_skeletonBosses[i].update(deltaTime, world, m_parentGame->getAudioEngine(), m_parentGame->getInputManager());

        if (type == ReturnType::DEATH)
        {
            BossSkeletonData bossData;
            bossData.position = { m_skeletonBosses[i].getPosition().x, m_skeletonBosses[i].getPosition().y };
            bossData.name = m_skeletonBosses[i].getName();

            m_deadSkeletonBosses.push_back(bossData);

            PlayerData data = m_player.getPlayerData();
            data.experience += m_skeletonBosses[i].getExperienceYield();
            m_player.setPlayerData(data);

            world->DestroyBody(m_skeletonBosses[i].getBody());
            m_skeletonBosses[i] = m_skeletonBosses.back();
            m_skeletonBosses.pop_back();
        }
        else if (type == ReturnType::COLLISION_PLAYER_PROJECTILE)
        {
            float strength = glm::pow(glm::log(m_player.getPlayerData().strength) * 3.0f, 1);
            float dexterity = glm::pow(glm::log(m_player.getPlayerData().dexterity) * 3.0f, 1);
            //printf("s: %f, d: %f\n", strength, dexterity);

            m_skeletonBosses[i].setHealth(m_skeletonBosses[i].getHealth() - (strength * dexterity));
            m_skeletonBosses[i].setHealth(m_skeletonBosses[i].getHealth() - (strength * dexterity));
            i++;
        }
        else
        {
            i++;
        }
    }

    if (m_parentGame->getInputManager().isKeyPressed(SDLK_k))
    {
        m_skeletonBosses[0].setHealth(0);
    }
}

void Map::updateNPCs(b2World* world, double deltaTime)
{
    for (auto& brek : m_breks)
    {
        ReturnType type = brek.update(deltaTime, world, m_parentGame->getAudioEngine(), m_parentGame->getInputManager());
        NPCActionReturnType action = brek.getActionType();

        if (type == ReturnType::COLLISION_PLAYER)
        {
            if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
            {
                brek.playDialogue();
                m_player.setUIState(UIState::UI);
            }
        }

        if (action == NPCActionReturnType::REMOVE_BOSS_GATE)
        {
            removeBossGate(world, "skeleton_boss_entrance");
            brek.setActionType(NPCActionReturnType::NONE);
        }

    }
}

void Map::updateItems(b2World* world, double deltaTime)
{
    auto items = m_inventory.getItems();

    if (!items.has_value())
        return;

    for (uint32_t i = 0; i < items.value().size(); i++)
    {
        if (!items.value()[i].inPlayerInventory())
        {
            ReturnType type = items.value()[i].update();

            if (type == ReturnType::COLLISION_PLAYER)
            {
                if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
                {
                    items.value()[i].setIsInPlayerInventory(true);
                    m_player.addToInventory(items.value()[i]);
                    m_inventory.removeById(items.value()[i].getId());
                }
            }
        }
    }
}

void Map::updateTriggers(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_oneTimeTriggers.size(); i++)
    {
        ReturnType type = m_oneTimeTriggers[i].update();

        if (type == ReturnType::COLLISION_PLAYER)
        {
            if (m_oneTimeTriggers[i].what() == "skeleton_boss_trigger")
            {
                if (m_skeletonBosses.size() > 0)
                {
                    m_skeletonBosses[0].showHealthBar();
                    m_parentGame->getAudioEngine().playSound("assets/audio/ogromne_cierpienie.ogg", { m_player.getPosition().x / PPM, m_player.getPosition().y / PPM, 0 });
                }
            }

            if (m_oneTimeTriggers[i].what() == "skeleton_boss_exit_trigger")
            {

            }

            if (m_oneTimeTriggers[i].what() == "level_2_discovery_trigger")
            {
                m_discoveredLevels.push_back("Second Area");
            }

            world->DestroyBody(m_oneTimeTriggers[i].getBody());
            m_oneTimeTriggers[i] = m_oneTimeTriggers.back();
            m_oneTimeTriggers.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void Map::updateRopes()
{
    for (uint32_t i = 0; i < m_ropes.size(); i++)
    {
        ReturnType type = m_ropes[i].update();

        if (type == ReturnType::COLLISION_PLAYER)
        {
            if (m_parentGame->getInputManager().isKeyPressed(SDLK_e))
            {
                m_player.attachToRope(i);
                m_ropes[i].attachPlayer();
            }
        }
    }
}

void Map::updateProjectiles(b2World* world, double deltaTime)
{
    for (uint32_t i = 0; i < m_projectiles.size(); i++)
    {
        ReturnType type = m_projectiles[i].update(deltaTime);

        if (type == ReturnType::COLLISION_PLAYER)
        {
            //printf("velocity %f, %f\n", m_projectiles[i].getBody()->GetLinearVelocity().x, m_projectiles[i].getBody()->GetLinearVelocity().y);
            if (glm::abs(m_projectiles[i].getBody()->GetLinearVelocity().x) > 1.5 || glm::abs(m_projectiles[i].getBody()->GetLinearVelocity().y) > 1.5)
            {
                //printf("velocity %f, %f\n", m_projectiles[i].getBody()->GetLinearVelocity().x, m_projectiles[i].getBody()->GetLinearVelocity().y);
                m_player.setHealth(m_player.getHealth() - 120);

                m_parentGame->getAudioEngine().playSound("assets/audio/hit.wav", { m_projectiles[i].getDestRect().x / PPM, m_projectiles[i].getDestRect().y / PPM, -2.0f });

                world->DestroyBody(m_projectiles[i].getBody());
                m_projectiles[i] = m_projectiles.back();
                m_projectiles.pop_back();
            }
            
        }
        //else
        //{
        //    i++;
        //}
    }

    for (uint32_t i = 0; i < m_playerProjectiles.size(); i++)
    {
        ReturnType type = m_playerProjectiles[i].update(deltaTime);

        if (type == ReturnType::COLLISION_SKELETON || type == ReturnType::COLLISION_BOSS_SKELETON || type == ReturnType::COLLISION_SPIDER)
        {
            m_parentGame->getAudioEngine().playSound("assets/audio/hit.wav", { m_playerProjectiles[i].getDestRect().x / PPM, m_playerProjectiles[i].getDestRect().y / PPM, -1.5f });

            world->DestroyBody(m_playerProjectiles[i].getBody());
            m_playerProjectiles[i] = m_playerProjectiles.back();
            m_playerProjectiles.pop_back();
        }
    }
}

void Map::drawDeadEnemies()
{
    for (uint32_t i = 0; i < m_deadSkeletons.size(); i++)
    {
        Engine::Renderer2D::DrawTexturedQuad(m_deadSkeletons[i].sprite.texture, 
            { m_deadSkeletons[i].sprite.position.x - m_deadSkeletons[i].sprite.dimentions.x / 2,
            m_deadSkeletons[i].sprite.position.y - m_deadSkeletons[i].sprite.dimentions.y / 2, m_deadSkeletons[i].sprite.dimentions},
            { 0.0f, 0.0f, 1.0f, 1.0f }, 1.0f, {255, 255, 255, 255}, false);
    }
}

void Map::removeBossGate(b2World* world, const std::string& gateId)
{
    for (uint32_t i = 0; i < m_bossRoomGates.size();)
    {
        if (m_bossRoomGates[i].name == gateId)
        {
            world->DestroyBody(m_bossRoomGates[i].body);
            m_bossRoomGates[i] = m_bossRoomGates.back();
            m_bossRoomGates.pop_back();

            Engine::Renderer2D::BeginScene(gateId);
            Engine::Renderer2D::EndScene(gateId);
        }
        else
        {
            i++;
        }
    }
}

void Map::onBossRoomGateParse(int32_t height, int32_t width, int32_t tileWidth, int32_t tileHeight, float scaling, const glm::vec2 position)
{
    /*
    //for (uint32_t i = 0; i < m_bossGateIds.size(); i++)
    //{
        Engine::Renderer2D::BeginScene(m_bossGateIds[m_id]);
        printf("gate name: %s, id: %d\n", m_bossGateIds[m_id].c_str(), m_id);

        //for (uint32_t i = 0; i < m_bossGateIds.size(); i++)
        //{
        //   printf("gate name: %s\n", m_bossGateIds[i].c_str());
        //}

        for (int32_t i = 0; i < m_map.GetNumTileLayers(); i++)
        {
            for (int32_t j = 0; j < height; j++)
            {
                m_pathFindingGrid[j].resize(width);
                for (int32_t k = 0; k < width; k++)
                {
                    int32_t gid = m_map.GetTileLayer(i)->GetTileGid(j, k);
                    int32_t tilesetIndex = m_map.FindTilesetIndex(gid);

                    if (tilesetIndex != -1)
                    {
                        if (m_map.GetTileLayer(i)->GetName() == "boss_room_gate")
                        {
                            glm::vec4 destRect((j * tileWidth * scaling) + position.x, (-k * tileHeight * scaling) + position.y, tileWidth * scaling, tileHeight * scaling);
                            std::string fileName = m_map.FindTileset(gid)->GetImage()->GetSource();
                            uint32_t tilesetWidth = m_map.FindTileset(gid)->GetColumns();
                            uint32_t tilesetHeight = m_map.FindTileset(gid)->GetTileCount() / m_map.FindTileset(gid)->GetColumns();

                            uint32_t firstGid = m_map.FindTileset(gid)->GetFirstGid();
                            uint32_t tileId = gid - firstGid;

                            glm::vec2 minPos = glm::vec2(floor(tileId % tilesetWidth) / (float)tilesetWidth, floor((float)tileId / (float)tilesetWidth) / (float)tilesetHeight);
                            glm::vec2 maxPos = glm::vec2((floor(minPos.x + (float)tileWidth) / (float)tileWidth) / (float)tilesetWidth, (floor(minPos.y + (float)tileHeight) / (float)tileHeight) / (float)tilesetHeight);

                            /*std::cout << "minPos( " << minPos.x << ", " << minPos.y << ")" << std::endl
                                        << "maxPos( " << maxPos.x << ", " << maxPos.y << ")" << std::endl;*//*

                            glm::vec4 uvRect(minPos.x, minPos.y, maxPos.x, maxPos.y);

                            Engine::Renderer2D::DrawTexturedQuad(fileName, destRect, uvRect, { 255, 255, 255, 255 }, false);
                        }
                    }
                }
            }
        }
        Engine::Renderer2D::EndScene(m_bossGateIds[m_id++]);*/
    //}*/
}

void Map::resetEnemyPositions(b2World* world)
{
    for (auto& skeleton : m_skeletons)
    {
        skeleton.setPosition(skeleton.getInitialPosition());
    }

    for (uint32_t i = 0; i < m_deadSkeletons.size(); i++)
    {
        Skeleton skeleton;
        skeleton.init(world, m_deadSkeletons[i].initialPosition);
        skeleton.collectGridData(m_pathFindingGrid);
        skeleton.collectPlayerData(&m_player);
        skeleton.collectProjectileData(&m_projectiles);
        m_skeletons.push_back(skeleton);
    }

    m_deadSkeletons.clear();

    for (auto& spider : m_spiders)
    {
        spider.setPosition(spider.getInitialPosition());
    }

    //for (uint32_t i = 0; i < m_deadSpiders.size(); i++)
    //{
    //    Spider spider;
    //    spider.init(world, m_deadSkeletons[i].initialPosition);
    //    spider.collectPlayerData(&m_player);
    //    m_spiders.push_back(spider);
    //}

    //m_deadSpiders.clear();
}

void Map::removeProjectiles(b2World* world)
{
    for (uint32_t i = 0; i < m_projectiles.size(); i++)
    {
        world->DestroyBody(m_projectiles[i].getBody());
    }
    m_projectiles.clear();

    for (uint32_t i = 0; i < m_playerProjectiles.size(); i++)
    {
        world->DestroyBody(m_playerProjectiles[i].getBody());
    }
    m_playerProjectiles.clear();
}

void Map::removeOutOfBoundsObjects(b2World* world)
{
    glm::vec2 mapDimentions = glm::vec2(m_map.GetWidth() * m_map.GetTileWidth() * m_scaling, m_map.GetHeight() * m_map.GetTileHeight() * m_scaling);
    int32_t padding = 5000;
    for (uint32_t i = 0; i < m_dynamicCrates.size();)
    {
        glm::vec2 pos = glm::vec2(m_dynamicCrates[i].getBody()->GetPosition().x * PPM, m_dynamicCrates[i].getBody()->GetPosition().y * PPM);
        if (pos.x < -padding || pos.x > mapDimentions.x + padding || pos.y > (mapDimentions.y / 2) + padding || pos.y < -(mapDimentions.y / 2) - padding)
        {
            world->DestroyBody(m_dynamicCrates[i].getBody());
            m_dynamicCrates[i] = m_dynamicCrates.back();
            m_dynamicCrates.pop_back();
        }
        else
        {
            i++;
        }
    }

    for (uint32_t i = 0; i < m_projectiles.size();)
    {
        glm::vec2 pos = glm::vec2(m_projectiles[i].getBody()->GetPosition().x * PPM, m_projectiles[i].getBody()->GetPosition().y * PPM);
        if (pos.x < -padding || pos.x > mapDimentions.x + padding || pos.y > (mapDimentions.y / 2) + padding || pos.y < -(mapDimentions.y / 2) - padding)
        {
            world->DestroyBody(m_projectiles[i].getBody());
            m_projectiles[i] = m_projectiles.back();
            m_projectiles.pop_back();
        }
        else
        {
            i++;
        }
    }

    for (uint32_t i = 0; i < m_playerProjectiles.size();)
    {
        glm::vec2 pos = glm::vec2(m_playerProjectiles[i].getBody()->GetPosition().x * PPM, m_playerProjectiles[i].getBody()->GetPosition().y * PPM);
        if (pos.x < -padding || pos.x > mapDimentions.x + padding || pos.y > (mapDimentions.y / 2) + padding || pos.y < -(mapDimentions.y / 2) - padding)
        {
            world->DestroyBody(m_playerProjectiles[i].getBody());
            m_playerProjectiles[i] = m_playerProjectiles.back();
            m_playerProjectiles.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void Map::resetEnemiesHealth()
{
    for (auto& skeleton : m_skeletonBosses)
    {
        skeleton.setHealth(skeleton.getMaxHealth());
    }

    for (auto& skeleton : m_skeletons)
    {
        skeleton.setHealth(skeleton.getMaxHealth());
    }

    for (auto& spider : m_spiders)
    {
        spider.setHealth(spider.getMaxHealth());
    }
}

void Map::drawChestInventory(Chest& chest)
{
    if (m_drawChestInventory)
    {
        m_player.setUIState(UIState::UI);
        m_player.setRestState(RestState::CHEST_OPENED);
        glm::vec2 dims = m_parentGame->getWindowDimentions() - glm::vec2(30);
        float ratio = dims.x / 1920;

        float padding = 15.0f;
        float imageSize = 100 * ratio;
        float buttonSize = 30 * ratio;

        ImGui::SetNextWindowPos({ 15, 15 }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ dims.x, dims.y }, ImGuiCond_Always);
        ImGui::Begin("inventory menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Columns(4, "inventory columns", false);
        ImGui::SetColumnWidth(0, dims.x * 0.3);
        ImGui::SetColumnWidth(1, dims.x * 0.34);
        ImGui::SetColumnWidth(2, dims.x * 0.3);
        ImGui::SetColumnWidth(3, dims.x * 0.06);

        ImGui::BeginChild(1, { (dims.x * 0.4f) - (padding * 2.0f), dims.y - (padding * 2.0f) }, false);

        auto items = chest.getItems();

        if (items.has_value())
        {
            uint32_t j = 0;
            // loop through all chest items
            for (uint32_t i = 0; i < items.value().size(); i++)
            {
                ImTextureID itemImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/" + items.value()[i].getImage()).id;
                ImTextureID useButtonImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/stone.png").id;

                ImGui::Image(itemImage, { imageSize - (padding * 2.0f), imageSize - (padding * 2.0f) });
                ImGui::SameLine();
                ImGui::Text(items.value()[i].getName().c_str()); ImGui::SameLine();

                ImGui::PushID(j++);
                ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
                ImGui::PopID();
                ImGui::PushID(j++);
                ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
                ImGui::PopID();
                ImGui::PushID(j++);
                if (ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }))
                {
                    // remove item and add it to player inventory
                    m_player.addToInventory(items.value()[i]);
                    m_chests[m_openedChestId].removeItemFromInventoryById(items.value()[i].getId());
                }
                ImGui::SameLine();
                ImGui::PopID();

                ImGui::PushID(j++);
                if (ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }))
                {
                    m_detailItem = i;
                }
                ImGui::PopID();

                ImGui::Text("Usage: %s", items.value()[i].getFor().c_str());

                ImGui::Dummy({ 1, 20 });
            }
        }
        else
        {
            ImGui::Text("Empty...");
        }

        ImGui::EndChild();
        ImGui::NextColumn();

        ImGui::BeginChild(2, { (dims.x * 0.54f) - (padding * 2.0f), dims.y - (padding * 2.0f) }, false);

        if (items.has_value())
        {
            if (items.value().size() > 0 && m_detailItem < items.value().size())
            {
                ImTextureID itemImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/" + items.value()[m_detailItem].getImage()).id;

                ImGui::Image(itemImage, { imageSize * 4.0f - (padding * 2.0f), imageSize * 4.0f - (padding * 2.0f) });

                ImGui::Text(items.value()[m_detailItem].getName().c_str());
                ImGui::Text("Uzycie: %s", items.value()[m_detailItem].getFor().c_str());
            }
        }

        ImGui::EndChild();

        ImGui::NextColumn();

        ImGui::BeginChild(3, { (dims.x * 0.54f) - (padding * 2.0f), dims.y - (padding * 2.0f) }, false);

        auto playerItems = m_player.getItems();

        if (playerItems.has_value())
        {
            uint32_t j = 0;
            // loop through all player items
            for (uint32_t i = 0; i < playerItems.value().size(); i++)
            {
                ImTextureID itemImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/" + playerItems.value()[i].getImage()).id;
                ImTextureID useButtonImage = (void*)(intptr_t)Engine::ResourceManager::getTexture("assets/textures/stone.png").id;

                ImGui::Image(itemImage, { imageSize - (padding * 2.0f), imageSize - (padding * 2.0f) });
                ImGui::SameLine();
                ImGui::Text(playerItems.value()[i].getName().c_str()); ImGui::SameLine();

                ImGui::PushID(j++);
                ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
                ImGui::PopID();
                ImGui::PushID(j++);
                ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }); ImGui::SameLine();
                ImGui::PopID();
                ImGui::PushID(j++);
                if (ImGui::ImageButton(useButtonImage, { buttonSize, buttonSize }))
                {
                    // remove item and add it to player inventory
                    m_chests[m_openedChestId].addToInventory(playerItems.value()[i]);
                    m_player.removeItemFromInventoryById(playerItems.value()[i].getId());
                }
                ImGui::PopID();

                ImGui::Text("Usage: %s", playerItems.value()[i].getFor().c_str());

                ImGui::Dummy({ 1, 20 });
            }
        }
        else
        {
            ImGui::Text("No items yet...");
        }

        ImGui::EndChild();

        ImGui::NextColumn();

        if (ImGui::Button("X", { 64 * ratio, 64 * ratio }))
        {
            chest.closeInventory();
            m_drawChestInventory = false;
            m_player.setUIState(UIState::NONE);
            m_player.setRestState(RestState::NONE);
        }

        ImGui::End();
    }
    
    if (!m_drawChestInventory && m_player.getRestState() == RestState::CHEST_OPENED)
    {
        m_player.setUIState(UIState::NONE);
        m_player.setRestState(RestState::NONE);
    }
}

std::optional<std::vector<Item>> Map::getChestItems(b2World* world, const std::string& filename)
{
    std::vector<Item> items;

    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());

    if (doc.Error())
    {
        printf("Error parsing save file: %s, filename: %s\n", doc.ErrorName(), filename.c_str());
        return {};
    }

    tinyxml2::XMLNode* node = doc.FirstChildElement("inventory");

    if (!node)
    {
        return {};
    }

    if (strcmp(node->Value(), "inventory") == 0)
    {
        tinyxml2::XMLElement* inventoryElement = node->ToElement();
        tinyxml2::XMLNode* inventoryNode = inventoryElement->FirstChild();

        while (inventoryNode)
        {

            if (strcmp(inventoryNode->Value(), "item") == 0)
            {
                tinyxml2::XMLElement* itemElement = inventoryNode->ToElement();
                tinyxml2::XMLNode* itemNode = itemElement->FirstChild();

                ItemData itemData;

                while (itemNode)
                {
                    if (strcmp(itemNode->Value(), "id") == 0)
                    {
                        tinyxml2::XMLElement* element = itemNode->ToElement();

                        itemData.id = element->Attribute("value");
                    }

                    if (strcmp(itemNode->Value(), "name") == 0)
                    {
                        tinyxml2::XMLElement* element = itemNode->ToElement();
                        itemData.name = element->Attribute("value");
                    }

                    if (strcmp(itemNode->Value(), "for") == 0)
                    {
                        tinyxml2::XMLElement* element = itemNode->ToElement();
                        itemData.purpose = element->Attribute("value");
                    }

                    if (strcmp(itemNode->Value(), "image") == 0)
                    {
                        tinyxml2::XMLElement* element = itemNode->ToElement();
                        itemData.image = element->Attribute("value");
                    }

                    if (strcmp(itemNode->Value(), "description") == 0)
                    {
                        tinyxml2::XMLElement* element = itemNode->ToElement();

                        itemData.description = Engine::String::Trim(element->GetText());
                    }

                    itemNode = itemNode->NextSibling();
                }

                Item item;
                item.init(world, itemData.id, itemData.name, itemData.purpose, itemData.image, itemData.description, itemData.destRect);
                items.push_back(item);
            }

            inventoryNode = inventoryNode->NextSibling();
        }
    }

    return items;
}

SaveData Map::collectSaveData()
{
    SaveData saveData;

    std::vector<ChestData> chests;
    std::vector<CrateData> crates;
    std::vector<SkeletonData> skeletons;
    std::vector<BossSkeletonData> skeletonBosses;
    std::vector<YieldedSoulsData> soulsData;
    std::vector<ItemData> playerItems;
    std::vector<ItemData> mapItems;
    std::vector<bool> bonfireData;

    for (uint32_t i = 0; i < m_chests.size(); i++)
    {
        ChestData data;
        data.closedTexture = m_chests[i].getClosedTextureFilename();
        data.destRect = m_chests[i].getDestRect();
        data.filtering = false;
        data.openedAtlasTexture = m_chests[i].getOpenedAtlasFilename();
        data.openedAtlasTextureMaxIndex = m_chests[i].getOpenedSprite().getMaxIndex();
        data.openedAtlasTextureRowsAndCols = m_chests[i].getOpenedSprite().getRowsAndCols();
        data.openedTexture = m_chests[i].getOpenedTextureFilename();
        data.openingAtlasTexture = m_chests[i].getOpeningAtlasFilename();
        data.openingAtlasTextureMaxIndex = m_chests[i].getOpeningSprite().getMaxIndex();
        data.openingAtlasTextureRowsAndCols = m_chests[i].getOpeningSprite().getRowsAndCols();

        auto items = m_chests[i].getItems();
        std::vector<ItemData> itemsData;

        if (items.has_value())
        {
            for (auto& item : items.value())
            {
                ItemData data;
                data.id = item.getId();
                data.name = item.getName();
                data.purpose = item.getFor();
                data.description = item.getDescription();
                data.image = item.getImage();
                data.destRect = item.getDestRect();
                itemsData.push_back(data);
            }
        }
        data.items = itemsData;

        chests.push_back(data);
    }

    for (uint32_t i = 0; i < m_dynamicCrates.size(); i++)
    {
        CrateData data;

        data.destRect = m_dynamicCrates[i].getDestRect();
        data.angle = m_dynamicCrates[i].getAngle();

        crates.push_back(data);
    }

    for (uint32_t i = 0; i < m_skeletons.size(); i++)
    {
        SkeletonData data;
        data.position = m_skeletons[i].getPosition();
        data.initialPosition = m_skeletons[i].getInitialPosition();

        skeletons.push_back(data);
    }

    for (uint32_t i = 0; i < m_skeletonBosses.size(); i++)
    {
        BossSkeletonData data;
        data.position = m_skeletonBosses[i].getPosition();
        data.name = m_skeletonBosses[i].getName();

        skeletonBosses.push_back(data);
    }

    for (uint32_t i = 0; i < m_soulsYielded.size(); i++)
    {
        YieldedSoulsData data;
        data.amount = m_soulsYielded[i].getAmount();
        data.pos = m_soulsYielded[i].getPosition();

        soulsData.push_back(data);
    }

    auto pItems = m_player.getItems();

    if (pItems.has_value())
    {
        for (uint32_t i = 0; i < pItems.value().size(); i++)
        {
            ItemData data;
            data.id = pItems.value()[i].getId();
            data.name = pItems.value()[i].getName();
            data.purpose = pItems.value()[i].getFor();
            data.description = pItems.value()[i].getDescription();
            data.image = pItems.value()[i].getImage();
            data.destRect = pItems.value()[i].getDestRect();

            playerItems.push_back(data);
        }
    }

    auto mItems = m_inventory.getItems();

    if (mItems.has_value())
    {
        for (uint32_t i = 0; i < mItems.value().size(); i++)
        {
            ItemData data;
            data.id = mItems.value()[i].getId();
            data.name = mItems.value()[i].getName();
            data.purpose = mItems.value()[i].getFor();
            data.description = mItems.value()[i].getDescription();
            data.image = mItems.value()[i].getImage();
            data.destRect = mItems.value()[i].getDestRect();

            mapItems.push_back(data);
        }
    }

    bonfireData.resize(m_bonfires.size());
    for (uint32_t i = 0; i < m_bonfires.size(); i++)
    {
        bonfireData[i] = m_bonfires[i].wasDiscovered();
    }

    saveData.playerPosition = m_player.getPosition();
    saveData.playerVelocity = glm::vec2(m_player.getBody()->GetLinearVelocity().x, m_player.getBody()->GetLinearVelocity().y);
    saveData.playerData = m_player.getPlayerData();
    saveData.playerType = m_player.getPlayerType();
    saveData.chests = chests;
    saveData.dynamicCrates = crates;
    saveData.skeletons = skeletons;
    saveData.deadSkeletons = m_deadSkeletons;
    saveData.deadSkeletonBosses = m_deadSkeletonBosses;
    saveData.skeletonBosses = skeletonBosses;
    saveData.bossRoomGates = m_bossRoomGates;
    saveData.souls = soulsData;
    saveData.lastBonfirePosition = m_player.getLastBonfireRestedAtPosition();
    saveData.playerItems = playerItems;
    saveData.mapItems = mapItems;
    saveData.bonfireData = bonfireData;

    return saveData;
}

void Map::loadScene(b2World* world)
{
    removeProjectiles(world);

    m_player.getBody()->SetTransform(b2Vec2(m_loadData.playerPosition.x / PPM, m_loadData.playerPosition.y / PPM), 0.0f);
    m_player.getBody()->SetLinearVelocity(b2Vec2(m_loadData.playerVelocity.x, m_loadData.playerVelocity.y));
    m_player.setPlayerData(m_loadData.playerData);
    m_player.setPlayerType(m_loadData.playerType);
    m_player.setLastBonfireRestedAtPosition(m_loadData.lastBonfirePosition);
    
    // Load crates
    for (uint32_t i = 0; i < m_dynamicCrates.size(); i++)
    {
        world->DestroyBody(m_dynamicCrates[i].getBody());
    }
    m_dynamicCrates.clear();

    for (uint32_t i = 0; i < m_loadData.dynamicCrates.size(); i++)
    {
        Crate crate;
        crate.init(world, { m_loadData.dynamicCrates[i].destRect.x, m_loadData.dynamicCrates[i].destRect.y }, { m_loadData.dynamicCrates[i].destRect.z, m_loadData.dynamicCrates[i].destRect.w });
        m_dynamicCrates.push_back(crate);
    }


    // Load chests

    for (uint32_t i = 0; i < m_chests.size(); i++)
    {
        world->DestroyBody(m_chests[i].getBody());
    }
    m_chests.clear();
    
    for (uint32_t i = 0; i < m_loadData.chests.size(); i++)
    {
        Chest chest;

        std::vector<Item> items;
        for (uint32_t j = 0; j < m_loadData.chests[i].items.size(); j++)
        {
            Item item;
            item.init(world, m_loadData.chests[i].items[j].id, m_loadData.chests[i].items[j].name, m_loadData.chests[i].items[j].purpose, m_loadData.chests[i].items[j].image,
                m_loadData.chests[i].items[j].description, m_loadData.chests[i].items[j].destRect);
            items.push_back(item);
        }

        chest.init(world, { m_loadData.chests[i].destRect.x, m_loadData.chests[i].destRect.y }, { m_loadData.chests[i].destRect.z, m_loadData.chests[i].destRect.w },
            m_loadData.chests[i].openingAtlasTexture, m_loadData.chests[i].openedAtlasTexture, m_loadData.chests[i].closedTexture, m_loadData.chests[i].openedTexture, items);
        m_chests.push_back(chest);
        
    }

    // Load Skeletons

    for (uint32_t i = 0; i < m_skeletons.size(); i++)
    {
        world->DestroyBody(m_skeletons[i].getBody());
    }
    m_skeletons.clear();

    for (uint32_t i = 0; i < m_loadData.skeletons.size(); i++)
    {
        Skeleton skeleton;
        skeleton.init(world, m_loadData.skeletons[i].position);
        skeleton.collectGridData(m_pathFindingGrid);
        skeleton.collectPlayerData(&m_player);
        skeleton.collectProjectileData(&m_projectiles);
        skeleton.setInitialPosition(m_loadData.skeletons[i].initialPosition);
        m_skeletons.push_back(skeleton);
    }

    m_deadSkeletons.clear();

    for (uint32_t i = 0; i < m_loadData.deadSkeletons.size(); i++)
    {
        DeadSkeleton skeleton;
        skeleton.initialPosition = m_loadData.deadSkeletons[i].initialPosition;
        skeleton.sprite = m_loadData.deadSkeletons[i].sprite;
        m_deadSkeletons.push_back(skeleton);
    }

    //  yielded souls

    m_soulsYielded.clear();

    for (uint32_t i = 0; i < m_loadData.souls.size(); i++)
    {
        SoulsYielded souls;
        souls.init(world, m_loadData.souls[i].pos, m_loadData.souls[i].amount);
        m_soulsYielded.push_back(souls);
    }

    // player items

    m_player.removeItems();

    for (uint32_t i = 0; i < m_loadData.playerItems.size(); i++)
    {
        Item item;

        std::string id = m_loadData.playerItems[i].id;
        std::string name = m_loadData.playerItems[i].name;
        std::string purpose = m_loadData.playerItems[i].purpose;
        std::string description = m_loadData.playerItems[i].description;
        std::string image = m_loadData.playerItems[i].image;

        item.init(world, id, name, purpose, image, description, { 0, 0, 0, 0 }, true);

        m_player.addToInventory(item);
    }

    // map items

    m_inventory.removeItems();

    for (uint32_t i = 0; i < m_loadData.mapItems.size(); i++)
    {
        Item item;

        std::string id = m_loadData.mapItems[i].id;
        std::string name = m_loadData.mapItems[i].name;
        std::string purpose = m_loadData.mapItems[i].purpose;
        std::string description = m_loadData.mapItems[i].description;
        std::string image = m_loadData.mapItems[i].image;
        glm::vec destRect = m_loadData.mapItems[i].destRect;

        item.init(world, id, name, purpose, image, description, destRect);

        m_inventory.addItem(item);
    }

    // bonfires

    for (uint32_t i = 0; i < m_loadData.bonfireData.size(); i++)
    {
        if (m_loadData.bonfireData[i])
        {
            m_bonfires[i].setDiscovered();
        }
    }

    for (uint32_t i = 0; i < m_loadData.deadSkeletonBosses.size(); i++)
    {
        for (uint32_t j = 0; j < m_skeletonBosses.size();)
        {
            if (m_loadData.deadSkeletonBosses[i].name == m_skeletonBosses[j].getName())
            {
                world->DestroyBody(m_skeletonBosses[j].getBody());

                m_skeletonBosses[j] = m_skeletonBosses.back();
                m_skeletonBosses.pop_back();
            }
            else
            {
                j++;
            }
        }
    }

    resetLoadData();
}

void Map::resetLoadData()
{
    m_loadData.bossRoomGates.clear();
    m_loadData.chests.clear();
    m_loadData.deadSkeletonBosses.clear();
    m_loadData.deadSkeletons.clear();
    m_loadData.dynamicCrates.clear();
    m_loadData.skeletonBosses.clear();
    m_loadData.skeletons.clear();
    m_loadData.souls.clear();
    m_loadData.playerItems.clear();
    m_loadData.mapItems.clear();
    m_loadData.bonfireData.clear();
}

/*glm::vec2 Map::mapToWorld(const glm::vec2& pos, float scaling, float tileHeight)
{
    glm::vec2 factor = glm::vec2(m_offset.x / tileHeight / scaling, glm::ceil(m_offset.y / tileHeight / scaling));

    //if (factor.x == 0.0f)
    //    factor.x = 1.0f;

    //if (factor.y == 0.0f)
    //    factor.y = 1.0f;

    //printf("Offset: %f, %f\n", factor.x, factor.y);

    return glm::vec2((pos.x * scaling) + (factor.x * tileHeight * scaling), -((pos.y * scaling) - (factor.y * tileHeight * scaling)));
}*/

void Map::save(Type type)
{
    m_saveData = collectSaveData();
    SaveManager::Save("save/save1.xml", m_saveData, type);
    printf("Game saved\n");
}

void Map::setActive(bool val)
{
    for (auto& crate : m_dynamicCrates)
    {
        crate.getBody()->SetAwake(false);
        crate.getBody()->SetActive(false);
    }

    for (auto& crate : m_staticCrates)
    {
        crate.getBody()->SetAwake(false);
        crate.getBody()->SetActive(false);
    }

    for (auto& spike : m_staticSpikes)
    {
        spike.getBody()->SetAwake(false);
        spike.getBody()->SetActive(false);
    }

    for (auto& bonfire : m_bonfires)
    {
        bonfire.getBody()->SetAwake(false);
        bonfire.getBody()->SetActive(false);
    }

    for (auto& skeleton : m_skeletons)
    {
        skeleton.getBody()->SetAwake(false);
        skeleton.getBody()->SetActive(false);
    }

    for (auto& boss : m_skeletonBosses)
    {
        boss.getBody()->SetAwake(false);
        boss.getBody()->SetActive(false);
    }

    for (auto& gate : m_bossRoomGates)
    {
        gate.getBody()->SetAwake(false);
        gate.getBody()->SetActive(false);
    }

    for (auto& chest : m_chests)
    {
        chest.getBody()->SetAwake(false);
        chest.getBody()->SetActive(false);
    }

    for (auto& proj : m_projectiles)
    {
        proj.getBody()->SetAwake(false);
        proj.getBody()->SetActive(false);
    }

    for (auto& proj : m_playerProjectiles)
    {
        proj.getBody()->SetAwake(false);
        proj.getBody()->SetActive(false);
    }

    for (auto& npc : m_breks)
    {
        npc.getBody()->SetAwake(false);
        npc.getBody()->SetActive(false);
    }

    for (auto& trigger : m_oneTimeTriggers)
    {
        trigger.getBody()->SetAwake(false);
        trigger.getBody()->SetActive(false);
    }

    for (auto& souls : m_soulsYielded)
    {
        souls.getBody()->SetAwake(false);
        souls.getBody()->SetActive(false);
    }

    for (auto& rope : m_ropes)
    {
        rope.setAwake(false);
        rope.setActive(false);
    }

    for (auto& collider : m_colliders)
    {
        collider->SetAwake(false);
        collider->SetActive(false);
    }
}
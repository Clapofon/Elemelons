#pragma once

#include "../Engine/Level.h"

#include "Crate.h"
#include "Spikes.h"
#include "Bonfire.h"
#include "Skeleton.h"
#include "Spider.h"
#include "Player.h"
#include "SkeletonBoss.h"
#include "Chest.h"
#include "Projectile.h"
#include "ContactListener.h"
#include "MapData.h"
#include "SaveManager.h"
#include "Brek.h"
#include "Rope.h"
//#include "RockSolid.h"

class Map : public Engine::Level
{
public:
	Map();
	virtual ~Map();

	virtual void onInit(b2World* world, float scaling, float tileHeight) override;
	virtual void onUpdate(b2World* world, double deltaTime) override;
	virtual void onDrawDynamic(Engine::ParticleSystem2D& particleSystem) override;
	virtual void onDrawStatic() override;

	virtual void onRectColliderParse(Engine::Tmx::Object* object, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onChainShapeColliderParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onObjectDynamicParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onObjectStaticParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onVegetationStaticParse(Engine::Tmx::ObjectGroup* group, float scaling) override;
	virtual void onAudioParse(Engine::Tmx::ObjectGroup* group, float scaling, uint32_t tileHeight) override;
	virtual void onEnemiesParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onPlayerParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onBossesParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onNPCsParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onItemsParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onTriggersParse(Engine::Tmx::ObjectGroup* group, b2World* world, float scaling, uint32_t tileHeight) override;
	virtual void onBossRoomGateParse(int32_t height, int32_t width, int32_t tileWidth, int32_t tileHeight, float scaling, const glm::vec2 position) override;

	void setActive(bool val);

	Player& getPlayer() { return m_player; }
	const std::vector<std::vector<int>>& getGrid() { return m_pathFindingGrid; }
	std::vector<Skeleton> getSkeletons() { return m_skeletons; }
	std::vector<Bonfire> getBonfires() { return m_bonfires; }
	std::vector<std::string> getDiscoveredAreas() { return m_discoveredLevels; }

	void removeBossGate(b2World* world, const std::string& gateId);

	void collectGameplayStateData(GameplayState state) { m_gameplayState = state; }

	void save(Type type = Type::ASYNC);

private:

	void updateDynamicObjects(b2World* world, double deltaTime);
	void updateEnemies(b2World* world, double deltaTime);
	void updateBosses(b2World* world, double deltaTime);
	void updateProjectiles(b2World* world, double deltaTime);
	void updateStaticObjects(b2World* world, double deltaTime);
	void updateNPCs(b2World* world, double deltaTime);
	void updateItems(b2World* world, double deltaTime);
	void updateTriggers(b2World* world, double deltaTime);
	void updateRopes();

	void updateAudioEngine(double deltaTime);

	void drawDynamicObjects();
	void drawStaticObjects();
	void drawEnemies(Engine::ParticleSystem2D& particleSystem);
	void drawDeadEnemies();
	void drawBosses(Engine::ParticleSystem2D& particleSystem);
	void drawNPCs(Engine::ParticleSystem2D& particleSystem);
	void drawItems();
	void drawRopes();

	void drawChestInventory(Chest& chest);

	void resetEnemyPositions(b2World* world);
	void resetEnemiesHealth();

	void removeProjectiles(b2World* world);
	void removeOutOfBoundsObjects(b2World* world);

	std::optional<std::vector<Item>> getChestItems(b2World* world, const std::string& filename);

	SaveData collectSaveData();
	void loadScene(b2World* world);

	void resetLoadData();

	//glm::vec2 mapToWorld(const glm::vec2& pos, float scaling, float tileHeight);

	Player m_player;
	glm::vec2 m_positionCurrentFrame = { 0, 0 }, m_positionLastFrame = { 0, 0 };

	std::vector<Crate> m_staticCrates;
	std::vector<Crate> m_dynamicCrates;
	std::vector<Spikes> m_staticSpikes;
	std::vector<Sprite> m_sprites;
	std::vector<Sprite> m_clouds;
	std::vector<Bonfire> m_bonfires;
	std::vector<Skeleton> m_skeletons;
	std::vector<Spider> m_spiders;
	std::vector<DeadSkeleton> m_deadSkeletons;
	std::vector<BossSkeletonData> m_deadSkeletonBosses;
	std::vector<SkeletonBoss> m_skeletonBosses;
	std::vector<BossRoomGate> m_bossRoomGates;
	std::vector<Chest> m_chests;
	std::vector<Projectile> m_projectiles;
	std::vector<Projectile> m_playerProjectiles;
	//std::vector<RockSolid> m_rocks;
	std::vector<Brek> m_breks;
	std::vector<Trigger> m_oneTimeTriggers;
	std::vector<SoulsYielded> m_soulsYielded;
	std::vector<Rope> m_ropes;
	std::vector<b2Body*> m_colliders;
	std::vector<std::string> m_discoveredLevels;

	ContactListener m_contactListener;

	GameplayState m_gameplayState = GameplayState::PLAY;

	SaveData m_saveData;
	SaveData m_loadData;
	bool m_awaitingLoad = false;
	bool m_playedDyingSound = false;
	Engine::Timer m_saveTimer;
	uint32_t m_detailItem = 0;
	bool m_drawChestInventory = false;
	uint32_t m_openedChestId = 0;

	uint32_t m_id = 0;

	Inventory m_inventory;
};


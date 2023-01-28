#pragma once

#include <tinyxml2.h>
#include <glm.hpp>

#include <future>
#include <vector>

#include "Player.h"
#include "MapData.h"
#include "EntityData.h"

struct SaveData
{
	bool loaded;

	glm::vec2 playerPosition;
	glm::vec2 playerVelocity;
	PlayerType playerType;
	PlayerData playerData;

	glm::vec2 lastBonfirePosition;

	std::vector<CrateData> dynamicCrates;
	std::vector<SkeletonData> skeletons;
	std::vector<DeadSkeleton> deadSkeletons;
	std::vector<BossSkeletonData> deadSkeletonBosses;
	std::vector<BossSkeletonData> skeletonBosses;
	std::vector<BossRoomGate> bossRoomGates;
	std::vector<ChestData> chests;
	std::vector<YieldedSoulsData> souls;
	std::vector<ItemData> playerItems;
	std::vector<ItemData> mapItems;
	std::vector<bool> bonfireData;
};

enum class Type
{
	ASYNC,
	DEFFERED
};

class SaveManager
{
public:
	static void Save(const std::string& filename, SaveData data, Type type = Type::ASYNC);
	static void Load(const std::string& filename, SaveData* data, Type type = Type::ASYNC);

	static bool Loaded() { return m_loaded; }

	static bool Saving() { return m_saving; }
	static bool Loading() { return m_loading; }

private:
	
	static void save(std::string filename, SaveData data);
	static void load(std::string filename, SaveData* data);

	static void parseNode(tinyxml2::XMLNode* n, SaveData* data);
	static void parsePlayerNode(tinyxml2::XMLNode* n, SaveData* data);
	static void parseMapNode(tinyxml2::XMLNode* n, SaveData* data);
	static void parseObjectNode(tinyxml2::XMLNode* n, SaveData* data);

	static std::vector<std::future<void>> m_futures;

	static bool m_loaded;
	static bool m_loading;
	static bool m_saving;
};


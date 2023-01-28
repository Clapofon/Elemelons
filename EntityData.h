#pragma once

#include <string>
#include <vector>

struct EntityData
{
	std::string type = "default";
	bool contact = false;
	std::string with = "default";
};

enum class AnimationState
{
	ATTACK,
	IDLE,
	WALK,
	DEATH
};

enum class ReturnType
{
	COLLISION_PLAYER,
	COLLISION_PLAYER_PROJECTILE,
	COLLISION_SKELETON,
	COLLISION_SPIDER,
	COLLISION_CRATE,
	COLLISION_SPIKES,
	COLLISION_BOSS_SKELETON,
	COLLISION_BONFIRE,
	DEATH,
	DYING,
	NONE
};

enum class NPCActionReturnType
{
	REMOVE_BOSS_GATE,
	NONE
};

struct ItemData
{
	std::string id;
	std::string name;
	std::string purpose;
	std::string image;
	std::string description;
	glm::vec4 destRect;
};

struct ChestData
{
	std::string openingAtlasTexture;
	std::string openedAtlasTexture;
	std::string openedTexture;
	std::string closedTexture;

	glm::vec4 destRect;

	glm::vec2 openedAtlasTextureRowsAndCols;
	uint32_t openedAtlasTextureMaxIndex;
	bool filtering;

	glm::vec2 openingAtlasTextureRowsAndCols;
	uint32_t openingAtlasTextureMaxIndex;

	std::vector<ItemData> items;
};

struct SkeletonData
{
	glm::vec2 position;
	glm::vec2 initialPosition;
};

struct BossSkeletonData
{
	glm::vec2 position;
	std::string name;
};

struct DeadBossSkeletonData
{
	glm::vec2 position;
	std::string name;
};

struct YieldedSoulsData
{
	glm::vec2 pos;
	uint64_t amount;
};

struct CrateData
{
	glm::vec4 destRect;
	float angle;
};

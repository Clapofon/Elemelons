#pragma once

#include "../Engine/InputManager.h"
#include "../Engine/PlayerConfig.h"
#include "../Engine/ParticleSystem2D.h"
#include "../Engine/Constants.h"

#include "EntityData.h"
#include "Bonfire.h"
#include "Projectile.h"
#include "Inventory.h"
#include "GameplayData.h"
#include "Rope.h"

#include <Box2D/Box2D.h>
#include <Box2D/Common/b2Settings.h>

const float PLAYER_WIDTH = 100;
const float MAX_SPEED = 4.0f;

enum class RestState
{
	SITTING,
	INVENTORY,
	TRAVEL,
	LEVELUP,
	NPC_DIALOGUE,
	CHEST_OPENED,
	NONE
};

enum class UIState
{
	UI,
	NONE
};

enum class PlayerAnimationState
{
	WALKING,
	RUNNING,
	IDLE,
	DEAD
};

enum class PlayerAttackState
{
	NONE,
	REGULAR
};

enum class PlayerType
{
	WATERMELON,
	EARTHMELON,
	FIREMELON,
	WINDMELON
};

struct PlayerData
{
	uint64_t experience = 0;
	uint32_t lvl = 10;
	int32_t vigor = 10;
	int32_t endurance = 10;
	int32_t strength = 10;
	int32_t dexterity = 10;
	int32_t luck = 10;
	int32_t itemDiscovery = 10;
	int32_t equipLoad = 10;
	int32_t maxStamina = 24;
	int32_t stamina = maxStamina;
	int32_t maxHealth = 60;
	int32_t health = maxHealth;
};

class Player
{
public:
	Player();
	~Player();

	void init(b2World* world, const glm::vec2& position, PlayerType type = PlayerType::WATERMELON);
	ReturnType update(b2World* world, Engine::InputManager& inputManager, double deltaTime, GameplayState state);
	void draw(Engine::ParticleSystem2D& particleSystem);

	void restAtBonfire(Bonfire bonfire);
	void leaveBonfire() { m_isAtBonfire = false; }
	bool isAtBonfire();

	void setMovementConfig(Engine::PlayerMovementConfig movementConfig);
	void setHealth(int32_t health) { m_playerData.health = health; }
	void setRestState(RestState state) { m_restState = state; }
	void setPlayerData(PlayerData data) { m_playerData = data; }
	void setPlayerType(PlayerType type) { m_type = type; }
	void setLastBonfireRestedAtPosition(const glm::vec2& position) { m_lastBonfireRestedAtPosition = position; }
	void setPosition(const glm::vec2& pos) { m_body->SetTransform({ pos.x / PPM, pos.y / PPM }, 0.0f); m_body->SetAwake(true); }
	void attachToRope(uint32_t id);
	void dettachFromRope(const b2Vec2& velocity);
	void setUIState(UIState state) { m_uiState = state; }

	void collectProjectileData(std::vector<Projectile>* projectiles) { m_projectiles = projectiles; }
	void collectCameraData(Engine::Camera* cam) { m_camera = cam; }

	RestState getRestState() { return m_restState; }
	glm::vec2 getPosition() { return glm::vec2(m_body->GetPosition().x * PPM, m_body->GetPosition().y * PPM); }
	glm::vec2 getDimentions() { return glm::vec2(PLAYER_WIDTH); }
	//Engine::AnimatedSprite& getIdleAtlas() { return m_idleSprite; }
	int32_t getHealth() { return m_playerData.health; }
	PlayerData getPlayerData() { return m_playerData; }
	PlayerAnimationState getAnimationState() { return m_animationState; }
	b2Body* getBody() { return m_body; }
	b2Fixture* getFixture() { return m_fixture; }
	uint32_t getMaxPossibleHealth() { return 60 + (glm::pow(glm::log(99) * 9.92, 2)); }
	uint32_t getMaxPossibleStamina() { return 60 + (glm::pow(glm::log(99) * 7, 2)); }
	PlayerType getPlayerType() { return m_type; }
	glm::vec2 getLastBonfireRestedAtPosition() { return m_lastBonfireRestedAtPosition; }
	bool isAttachedToRope() { return m_attachedToRope; }
	uint32_t getRopeIndex() { return m_ropeId; }
	bool canDettach() { return m_canDettach; }
	UIState getUIState() { return m_uiState; }

	void addToInventory(Item& item);
	void removeItemFromInventoryById(const std::string& id);
	void removeItemFromInventoryByName(const std::string& name);
	void removeItemsFromInventoryByName(const std::string& name, uint32_t count);
	void removeAllItemsFromInventoryByName(const std::string& name);
	void removeItems();

	Inventory& getInventory() { return m_inventory; }
	std::optional<std::vector<Item>> getItemsByName(const std::string& name);
	std::optional<Item> getItemById(const std::string& id);
	std::optional<std::vector<Item>> getItems();


private:
	void move(Engine::InputManager& inputManager, double deltaTime);
	void shootProjectile(b2World* world, Engine::InputManager& inputManager, const glm::vec2& size, const glm::vec2& colliderSize, const glm::vec2& power, glm::vec2 offset);

	Engine::AnimatedSprite m_idleWatermelonSprite;
	Engine::AnimatedSprite m_deadWatermelonSprite;

	Engine::AnimatedSprite m_idleFiremelonSprite;
	Engine::AnimatedSprite m_deadFiremelonSprite;

	Engine::AnimatedSprite m_handSprite;

	PlayerAnimationState m_animationState = PlayerAnimationState::WALKING;
	PlayerAttackState m_attackState = PlayerAttackState::REGULAR;
	PlayerType m_type = PlayerType::WATERMELON;
	UIState m_uiState = UIState::NONE;

	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;

	Engine::PlayerMovementConfig m_movementConfig;

	std::shared_ptr<EntityData> m_data;
	glm::vec2 m_lastBonfireRestedAtPosition = { 0, 0 };

	bool m_canAttack = false;
	bool m_canStaminaRegen = false;
	bool m_handFlip = false;
	bool m_isAtBonfire = false;

	RestState m_restState = RestState::NONE;
	Engine::Timer m_attackTimer;
	Engine::Timer m_staminaTimer;
	std::vector<Projectile>* m_projectiles = nullptr;
	Engine::Camera* m_camera = nullptr;

	Inventory m_inventory;

	PlayerData m_playerData;

	bool m_attachedToRope = false;
	uint32_t m_ropeId = 0;
	bool m_canDettach = false;
	uint32_t m_counter = 0;
};
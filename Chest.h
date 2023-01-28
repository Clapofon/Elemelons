#pragma once

#include "../Engine/Object.h"
#include "../Engine/AnimatedSprite.h"

#include "EntityData.h"
#include "Inventory.h"

enum class ChestState
{
	CLOSED,
	OPENING,
	OPENED_FULL,
	OPENED_EMPTY
};

class Chest : public Engine::Object
{
public:
	Chest();
	~Chest();

	virtual void init(b2World* world, const glm::vec2& position, const glm::vec2& dimentions, const std::string& openingAtlas, const std::string& openedAtlas,
		const std::string& closedTexture, const std::string& openedTexture, std::vector<Item>& items);
	virtual void draw();
	ReturnType update(double deltaTime);

	void setState(ChestState state) { m_state = state; }

	ChestState getState() { return m_state; }

	std::string getOpeningAtlasFilename() { return m_sprite.getFilename(); }
	std::string getOpenedTextureFilename() { return m_openedTexture; }
	std::string getClosedTextureFilename() { return m_closedTexture; }
	std::string getOpenedAtlasFilename() { return m_spriteOpened.getFilename(); }

	Engine::AnimatedSprite getOpenedSprite() { return m_spriteOpened; }
	Engine::AnimatedSprite getOpeningSprite() { return m_sprite; }

	void openInventory() { m_isInventoryOpened = true; }
	void closeInventory() { m_isInventoryOpened = false; }
	bool isInventoryOpened() { return m_isInventoryOpened; }

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
	Engine::AnimatedSprite m_sprite;
	Engine::AnimatedSprite m_spriteOpened;

	std::string m_closedTexture;
	std::string m_openedTexture;

	ChestState m_state = ChestState::CLOSED;

	std::shared_ptr<EntityData> m_data;

	Inventory m_inventory;
	bool m_isInventoryOpened = false;
};


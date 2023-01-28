#pragma once

#include "Item.h"

#include <vector>
#include <optional>

class Inventory
{
public:
	Inventory();
	~Inventory();

	void init();

	void addItem(Item& item);
	void addItems(std::vector<Item>& items);
	std::optional<std::vector<Item>> getItemsByName(const std::string& name);
	std::optional<Item> getItemById(const std::string& id);
	std::optional<std::vector<Item>> getItems();

	bool isEmpty() { return !m_items.size(); }

	void removeById(const std::string& id);
	void removeByName(const std::string& name);
	void removeByName(const std::string& name, uint32_t count);
	void removeAllByName(const std::string& name);
	void removeItems();

private:
	std::vector<Item> m_items;
};


#include "Inventory.h"

Inventory::Inventory()
{

}

Inventory::~Inventory()
{

}

void Inventory::init()
{

}

void Inventory::addItem(Item& item)
{
	m_items.push_back(item);
}

void Inventory::addItems(std::vector<Item>& items)
{
	for (auto& item : items)
	{
		m_items.push_back(item);
	}
}

std::optional<std::vector<Item>> Inventory::getItemsByName(const std::string& name)
{
	std::vector<Item> items;
	for (uint32_t i = 0; i < m_items.size(); i++)
	{
		if (m_items[i].getName() == name)
		{
			items.push_back(m_items[i]);
		}
	}

	if (items.size() > 0)
		return items;

	return {};
}

std::optional<Item> Inventory::getItemById(const std::string& id)
{
	for (uint32_t i = 0; i < m_items.size(); i++)
	{
		if (m_items[i].getId() == id)
		{
			return m_items[i];
		}
	}

	return {};
}

void Inventory::removeById(const std::string& id)
{
	for (uint32_t i = 0; i < m_items.size(); i++)
	{
		if (m_items[i].getId() == id)
		{
			m_items[i] = m_items.back();
			m_items.pop_back();
			return;
		}
	}
}

void Inventory::removeByName(const std::string& name)
{
	for (uint32_t i = 0; i < m_items.size(); i++)
	{
		if (m_items[i].getName() == name)
		{
			m_items[i] = m_items.back();
			m_items.pop_back();
			return;
		}
	}
}

void Inventory::removeByName(const std::string& name, uint32_t count)
{
	uint32_t removed = 0;
	for (uint32_t i = 0; i < m_items.size();)
	{
		if (removed >= count)
		{
			return;
		}

		if (m_items[i].getName() == name)
		{
			m_items[i] = m_items.back();
			m_items.pop_back();
			
			removed++;
		}
		else
		{
			i++;
		}
	}
}

void Inventory::removeAllByName(const std::string& name)
{
	for (uint32_t i = 0; i < m_items.size();)
	{
		if (m_items[i].getName() == name)
		{
			m_items[i] = m_items.back();
			m_items.pop_back();
		}
		else 
		{
			i++;
		}
	}
}

void Inventory::removeItems()
{
	m_items.clear();
}

std::optional<std::vector<Item>> Inventory::getItems()
{
	if (m_items.size() > 0)
		return m_items;

	return {};
}
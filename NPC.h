#pragma once

#include "Actor.h"
#include "Enemy.h"
#include "Dialogue.h"
#include "EntityData.h"
#include "Inventory.h"
#include "Player.h"

#include <tinyxml2.h>


class NPC : public Actor
{
public:

	NPC() {}
	virtual ~NPC() {}

	virtual void init(b2World* world, const glm::vec2& position) = 0;
	virtual void onInit() = 0;
	virtual void draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims) = 0;
	virtual ReturnType update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager) = 0;

	void collectPlayerData(Player* player) { m_player = player; }

	b2Body* getBody() { return m_body; }

	void loadDialogue(const std::string& filename)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());

		if (doc.Error())
		{
			printf("Save file not found\n");
			return;
		}

		std::vector<std::string> lines;
		std::vector<float> times;
		std::vector<std::string> audio;

		tinyxml2::XMLNode* dialogueListNode = doc.FirstChildElement("dialoguelist");
		tinyxml2::XMLElement* elem = dialogueListNode->ToElement();

		tinyxml2::XMLNode* node = elem->FirstChild();
		while (node)
		{
			if (strcmp(node->Value(), "part") == 0)
			{
				tinyxml2::XMLElement* pElement = node->ToElement();
				tinyxml2::XMLNode* pNode = pElement->FirstChild();

				std::string partName = pElement->Attribute("name");

				while (pNode)
				{
					if (strcmp(pNode->Value(), "dialogue") == 0)
					{

						tinyxml2::XMLElement* dElement = pNode->ToElement();
						tinyxml2::XMLNode* dNode = dElement->FirstChild();

						while (dNode)
						{
							if (strcmp(dNode->Value(), "line") == 0)
							{
								tinyxml2::XMLElement* elem = dNode->ToElement();

								lines.push_back(elem->Attribute("value"));
							}

							if (strcmp(dNode->Value(), "time") == 0)
							{
								tinyxml2::XMLElement* elem = dNode->ToElement();

								times.push_back(elem->FloatAttribute("value"));
							}

							if (strcmp(dNode->Value(), "audio") == 0)
							{
								tinyxml2::XMLElement* elem = dNode->ToElement();

								audio.push_back(elem->Attribute("value"));
							}

							dNode = dNode->NextSibling();
						}
					}

					pNode = pNode->NextSibling();
				}
				
				Dialogue dialogue;
				dialogue.init(lines, audio, times);

				m_dialogues.insert(std::make_pair(partName, dialogue));

				lines.clear();
				times.clear();
				audio.clear();
			}

			node = node->NextSibling();
		}
	}

	void addToInventory(Item item)
	{
		m_inventory.addItem(item);
	}

	void removeItemFromInventoryById(const std::string& id)
	{
		m_inventory.removeById(id);
	}

	void removeItemFromInventoryByName(const std::string& name)
	{
		m_inventory.removeByName(name);
	}

	std::optional<std::vector<Item>> getItemsByName(const std::string& name)
	{
		return m_inventory.getItemsByName(name);
	}

	std::optional<Item> getItemById(const std::string& id)
	{
		return m_inventory.getItemById(id);
	}

	std::optional<std::vector<Item>> getItems()
	{
		return m_inventory.getItems();
	}

protected:

	Dialogue& getDialogue(const std::string& dialogueName)
	{
		auto iterator = m_dialogues.find(dialogueName);

		if (iterator != m_dialogues.end())
		{
			return iterator->second;
		}
		else
		{
			printf("Dialogue with name \"%s\" does not exist.\n", dialogueName.c_str());
		}
	}

	std::shared_ptr<EntityData> m_entityData;
	NPCActionReturnType m_actionReturnType = NPCActionReturnType::NONE;

	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;

	std::unordered_map<std::string, Dialogue> m_dialogues;

	Inventory m_inventory;

	Player* m_player = nullptr;
};


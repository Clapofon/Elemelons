#include "SaveManager.h"

#include "../Engine/StringUtil.h"

#include <fstream>
#include <iomanip>

std::vector<std::future<void>> SaveManager::m_futures;
bool SaveManager::m_loaded = false;
bool SaveManager::m_loading = false;
bool SaveManager::m_saving = false;
static std::mutex s_loadMutex;

void SaveManager::save(std::string filename, SaveData data)
{
	m_saving = true;

	std::ofstream file;
	file.open(filename, std::fstream::trunc);

	//file << std::setprecision(6);
	file.setf(std::ios_base::fixed);
	file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	file << "<save>\n";

	file << "	<player>\n";
	file << "		<position x=\"" << data.playerPosition.x << "\" y=\"" << data.playerPosition.y << "\" />\n";
	file << "		<velocity x=\"" << data.playerVelocity.x << "\" y=\"" << data.playerVelocity.y << "\" />\n";
	file << "		<bonfire x=\"" << data.lastBonfirePosition.x << "\" y=\"" << data.lastBonfirePosition.y << "\" />\n";

	switch (data.playerType)
	{
	case PlayerType::WATERMELON:
		file << "		<type value=\"watermelon\"/>\n";
		break;
	case PlayerType::EARTHMELON:
		file << "		<type value=\"earthmelon\"/>\n";
		break;
	case PlayerType::FIREMELON:
		file << "		<type value=\"firemelon\"/>\n";
		break;
	case PlayerType::WINDMELON:
		file << "		<type value=\"windmelon\"/>\n";
		break;
	default:
		file << "		<type value=\"watermelon\"/>\n";
		break;
	}

	file << "		<data>\n";

	file << "			<experience value=\"" << data.playerData.experience << "\" />\n";
	file << "			<lvl value=\"" << data.playerData.lvl << "\" />\n";
	file << "			<vigor value=\"" << data.playerData.vigor << "\" />\n";
	file << "			<endurance value=\"" << data.playerData.endurance << "\" />\n";
	file << "			<strength value=\"" << data.playerData.strength << "\" />\n";
	file << "			<dextrity value=\"" << data.playerData.dexterity << "\" />\n";
	file << "			<luck value=\"" << data.playerData.luck << "\" />\n";
	file << "			<itemDiscovery value=\"" << data.playerData.itemDiscovery << "\" />\n";
	file << "			<equipLoad value=\"" << data.playerData.equipLoad << "\" />\n";
	file << "			<maxStamina value=\"" << data.playerData.maxStamina << "\" />\n";
	file << "			<stamina value=\"" << data.playerData.stamina << "\" />\n";
	file << "			<maxHealth value=\"" << data.playerData.maxHealth << "\" />\n";
	file << "			<health value=\"" << data.playerData.health << "\" />\n";

	file << "		</data>\n";
	file << "		<inventory>\n";
	
	for (uint32_t i = 0; i < data.playerItems.size(); i++)
	{
		file << "			<item>\n";
		file << "				<id value=\"" << data.playerItems[i].id << "\" />\n";
		file << "				<name value=\"" << data.playerItems[i].name << "\" />\n";
		file << "				<for value=\"" << data.playerItems[i].purpose << "\" />\n";
		file << "				<image value=\"" << data.playerItems[i].image << "\" />\n";
		file << "				<description>\n";
		file << "					" << data.playerItems[i].description << "\n";
		file << "				</description>\n";
		file << "			</item>\n";
	}

	file << "		</inventory>\n";
	file << "	</player>\n";
	file << "	<map>\n";
	file << "		<inventory>\n";

	for (uint32_t i = 0; i < data.mapItems.size(); i++)
	{
		file << "			<item>\n";
		file << "				<id value=\"" << data.mapItems[i].id << "\" />\n";
		file << "				<name value=\"" << data.mapItems[i].name << "\" />\n";
		file << "				<for value=\"" << data.mapItems[i].purpose << "\" />\n";
		file << "				<image value=\"" << data.mapItems[i].image << "\" />\n";
		file << "				<destRect x=\"" << data.mapItems[i].destRect.x << "\" y=\"" << data.mapItems[i].destRect.y 
												<< "\" width=\"" << data.mapItems[i].destRect.z 
												<< "\" height=\"" << data.mapItems[i].destRect.w << "\" />\n";
		file << "				<description>\n";
		file << "					" << data.mapItems[i].description << "\n";
		file << "				</description>\n";
		file << "			</item>\n";
	}

	file << "		</inventory>\n";
	
	for (uint32_t i = 0; i < data.bonfireData.size(); i++)
	{
		if (data.bonfireData[i])
		{
			file << "		<bonfire discovered=\"true\" />\n";
		}
		else
		{
			file << "		<bonfire discovered=\"false\" />\n";
		}
	}

	file << "	</map>\n";

	for (uint32_t i = 0; i < data.dynamicCrates.size(); i++)
	{
		file << "	<object name=\"crate\">\n";
		file << "		<position x=\"" << data.dynamicCrates[i].destRect.x << "\" y=\"" << data.dynamicCrates[i].destRect.y << "\" />\n";
		file << "		<dimentions width=\"" << data.dynamicCrates[i].destRect.z << "\" height=\"" << data.dynamicCrates[i].destRect.w << "\" />\n";
		file << "		<rotation angle=\"" << data.dynamicCrates[i].angle << "\" />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.skeletons.size(); i++)
	{
		file << "	<object name=\"skeleton\">\n";
		file << "		<position x=\"" << data.skeletons[i].position.x << "\" y=\"" << data.skeletons[i].position.y << "\" />\n";
		file << "		<initialPosition x=\"" << data.skeletons[i].initialPosition.x << "\" y=\"" << data.skeletons[i].initialPosition.y << "\" />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.deadSkeletons.size(); i++)
	{
		file << "	<object name=\"deadSkeleton\">\n";
		file << "		<position x=\"" << data.deadSkeletons[i].sprite.position.x << "\" y=\"" << data.deadSkeletons[i].sprite.position.y << "\" />\n";
		file << "		<initialPosition x=\"" << data.deadSkeletons[i].initialPosition.x << "\" y=\"" << data.deadSkeletons[i].initialPosition.y << "\" />\n";
		file << "		<dimentions width=\"" << data.deadSkeletons[i].sprite.dimentions.x << "\" height=\"" << data.deadSkeletons[i].sprite.dimentions.y << "\" />\n";
		file << "		<texture path=\"" << data.deadSkeletons[i].sprite.texture << "\" />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.deadSkeletonBosses.size(); i++)
	{
		file << "	<object name=\"deadSkeletonBoss\">\n";
		file << "		<position x=\"" << data.deadSkeletonBosses[i].position.x << "\" y=\"" << data.deadSkeletonBosses[i].position.y << "\" />\n";
		file << "		<name value=\"" << data.deadSkeletonBosses[i].name << "\" />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.skeletonBosses.size(); i++)
	{
		file << "	<object name=\"skeletonBoss\">\n";
		file << "		<position x=\"" << data.skeletonBosses[i].position.x << "\" y=\"" << data.skeletonBosses[i].position.y << "\" />\n";
		file << "		<name value=\"" << data.skeletonBosses[i].name << "\" />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.souls.size(); i++)
	{
		file << "	<object name=\"souls\">\n";
		file << "		<position x=\"" << data.souls[i].pos.x << "\" y=\"" << data.souls[i].pos.y << "\" />\n";
		file << "		<amount value=\"" << data.souls[i].amount << "\"" << " />\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.bossRoomGates.size(); i++)
	{
		file << "	<object name=\"bossRoomGate\" gateName=\"" << data.bossRoomGates[i].name << "\">\n";
		file << "		<points>\n";
		
		for (uint32_t j = 0; j < data.bossRoomGates[i].points.size(); j++)
		{
			file << "			<point x=\"" << data.bossRoomGates[i].points[j].x << "\" y=\"" << data.bossRoomGates[i].points[j].y << "\" />\n";
		}

		file << "		</points>\n";
		file << "	</object>\n";
	}

	for (uint32_t i = 0; i < data.chests.size(); i++)
	{
		file << "	<object name=\"chest\">\n";
		file << "		<position x=\"" << data.chests[i].destRect.x<< "\" y=\"" << data.chests[i].destRect.y << "\" />\n";
		file << "		<dimentions width=\"" << data.chests[i].destRect.z << "\" height=\"" << data.chests[i].destRect.w << "\" />\n";
		file << "		<texture type=\"texture\" name=\"opened\" filename=\"" << data.chests[i].openedTexture << "\" />\n";
		file << "		<texture type=\"texture\" name=\"closed\" filename=\"" << data.chests[i].closedTexture << "\" />\n";
		file << "		<atlas>\n";
		file << "			<texture name=\"opened\" filename=\"" << data.chests[i].openedAtlasTexture << "\" />\n";
		file << "			<properties>\n";
		file << "				<property name=\"rowsAndCols\" x=\"" << data.chests[i].openedAtlasTextureRowsAndCols.x << "\" y =\"" << data.chests[i].openedAtlasTextureRowsAndCols.y << "\" />\n";
		file << "				<property name=\"maxIndex\" value=\"" << data.chests[i].openedAtlasTextureMaxIndex << "\" />\n";
		
		bool val = data.chests[i].filtering;
		if (val)
		{
			file << "				<property name=\"filtering\" value=\"true\" />\n";
		}
		else
		{
			file << "				<property name=\"filtering\" value=\"false\" />\n";
		}

		file << "			</properties>\n";
		file << "		</atlas>\n";
		file << "		<atlas>\n";
		file << "		<texture type=\"atlas\" name=\"opening\" filename=\"" << data.chests[i].openingAtlasTexture << "\" />\n";
		file << "			<properties>\n";
		file << "				<property name=\"rowsAndCols\" x=\"" << data.chests[i].openingAtlasTextureRowsAndCols.x << "\" y =\"" << data.chests[i].openingAtlasTextureRowsAndCols.y << "\" />\n";
		file << "				<property name=\"maxIndex\" value=\"" << data.chests[i].openingAtlasTextureMaxIndex << "\" />\n";
		
		val = data.chests[i].filtering;
		if (val)
		{
			file << "				<property name=\"filtering\" value=\"true\" />\n";
		}
		else
		{
			file << "				<property name=\"filtering\" value=\"false\" />\n";
		}

		file << "			</properties>\n";
		file << "		</atlas>\n";
		file << "		<inventory>\n";

		for (uint32_t j = 0; j < data.chests[i].items.size(); j++)
		{
			file << "			<item>\n";
			file << "				<id value=\"" << data.chests[i].items[j].id << "\" />\n";
			file << "				<name value=\"" << data.chests[i].items[j].name << "\" />\n";
			file << "				<for value=\"" << data.chests[i].items[j].purpose << "\" />\n";
			file << "				<image value=\"" << data.chests[i].items[j].image << "\" />\n";
			file << "				<description>\n";
			file << "					" << data.chests[i].items[j].description << "\n";
			file << "				</description>\n";
			file << "			</item>\n";
		}

		file << "		</inventory>\n";
		file << "	</object>\n";
	}

	file << "</save>\n";
	file.close();

	m_saving = false;
}

void SaveManager::load(std::string filename, SaveData* data)
{
	m_loaded = false;
	m_loading = true;

	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());

	if (doc.Error())
	{
		printf("Error parsing save file: %s\n", doc.ErrorName());
		m_loaded = false;
		m_loading = false;
		return;
	}

	tinyxml2::XMLNode* saveNode = doc.FirstChildElement("save");

	if (!saveNode)
	{
		data->loaded = false;
		m_loaded = false;
		m_loading = false;
		return;
	}

	std::lock_guard<std::mutex> lock(s_loadMutex);

	parseNode(saveNode, data);
	
	m_loading = false;
	m_loaded = true;

	data->loaded = true;
}

void SaveManager::Save(const std::string& filename, SaveData data, Type type)
{
	if (type == Type::ASYNC)
	{
		m_futures.push_back(std::async(std::launch::async, save, filename, data));
	}
	
	if (type == Type::DEFFERED)
	{
		save(filename, data);
	}
}

void SaveManager::Load(const std::string& filename, SaveData* data, Type type)
{
	if (type == Type::ASYNC)
	{
		m_futures.push_back(std::async(std::launch::async, load, filename, data));
	}
	
	if (type == Type::DEFFERED)
	{
		load(filename, data);
	}
}

void SaveManager::parseNode(tinyxml2::XMLNode* n, SaveData* data)
{
	tinyxml2::XMLElement* elem = n->ToElement();

	tinyxml2::XMLNode* node = elem->FirstChild();
	while (node)
	{
		if (strcmp(node->Value(), "player") == 0)
		{
			parsePlayerNode(node, data);
		}

		if (strcmp(node->Value(), "map") == 0)
		{
			parseMapNode(node, data);
		}

		if (strcmp(node->Value(), "object") == 0)
		{
			parseObjectNode(node, data);
		}

		node = node->NextSibling();
	}
}

void SaveManager::parsePlayerNode(tinyxml2::XMLNode* n, SaveData* data)
{
	tinyxml2::XMLElement* elem = n->ToElement();
	tinyxml2::XMLNode* node = elem->FirstChild();

	while (node)
	{
		if (strcmp(node->Value(), "position") == 0)
		{
			tinyxml2::XMLElement* positionElement = node->ToElement();
			data->playerPosition = glm::vec2(positionElement->FloatAttribute("x"), positionElement->FloatAttribute("y"));
		}

		if (strcmp(node->Value(), "velocity") == 0)
		{
			tinyxml2::XMLElement* velocityElement = node->ToElement();
			data->playerVelocity = glm::vec2(velocityElement->FloatAttribute("x"), velocityElement->FloatAttribute("y"));
		}

		if (strcmp(node->Value(), "bonfire") == 0)
		{
			tinyxml2::XMLElement* element = node->ToElement();
			data->lastBonfirePosition = glm::vec2(element->FloatAttribute("x"), element->FloatAttribute("y"));
		}

		if (strcmp(node->Value(), "type") == 0)
		{
			tinyxml2::XMLElement* typeElement = node->ToElement();
			
			std::string playerType = typeElement->Attribute("value");

			if (playerType == "watermelon")
			{
				data->playerType = PlayerType::WATERMELON;
			}

			if (playerType == "earthmelon")
			{
				data->playerType = PlayerType::EARTHMELON;
			}

			if (playerType == "windmelon")
			{
				data->playerType = PlayerType::WINDMELON;
			}

			if (playerType == "firemelon")
			{
				data->playerType = PlayerType::FIREMELON;
			}
		}

		if (strcmp(node->Value(), "data") == 0)
		{
			tinyxml2::XMLElement* dataElement = node->ToElement();
			tinyxml2::XMLNode* dataNode = dataElement->FirstChild();

			while (dataNode)
			{
				if (strcmp(dataNode->Value(), "experience") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.experience = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "lvl") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.lvl = element->FloatAttribute("value");
				}

				if (strcmp(dataNode->Value(), "vigor") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.vigor = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "endurance") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.endurance = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "strength") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.strength = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "dexterity") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.dexterity = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "luck") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.luck = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "itemDiscovery") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.itemDiscovery = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "equipLoad") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.equipLoad = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "maxStamina") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.maxStamina = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "stamina") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.stamina = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "maxHealth") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.maxHealth = element->IntAttribute("value");
				}

				if (strcmp(dataNode->Value(), "health") == 0)
				{
					tinyxml2::XMLElement* element = dataNode->ToElement();
					data->playerData.health = element->IntAttribute("value");
				}

				dataNode = dataNode->NextSibling();
			}
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

					data->playerItems.push_back(itemData);
				}

				inventoryNode = inventoryNode->NextSibling();
			}
		}

		node = node->NextSibling();
	}
}

void SaveManager::parseMapNode(tinyxml2::XMLNode* n, SaveData* data)
{
	tinyxml2::XMLElement* elem = n->ToElement();
	tinyxml2::XMLNode* node = elem->FirstChild();

	while (node)
	{

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

						if (strcmp(itemNode->Value(), "destRect") == 0)
						{
							tinyxml2::XMLElement* element = itemNode->ToElement();
							itemData.destRect.x = element->FloatAttribute("x");
							itemData.destRect.y = element->FloatAttribute("y");
							itemData.destRect.z = element->FloatAttribute("width");
							itemData.destRect.w = element->FloatAttribute("height");
						}

						if (strcmp(itemNode->Value(), "description") == 0)
						{
							tinyxml2::XMLElement* element = itemNode->ToElement();

							itemData.description = Engine::String::Trim(element->GetText());
						}

						itemNode = itemNode->NextSibling();
					}

					data->mapItems.push_back(itemData);
				}

				inventoryNode = inventoryNode->NextSibling();
			}
		}

		if (strcmp(node->Value(), "bonfire") == 0)
		{
			tinyxml2::XMLElement* element = node->ToElement();
			data->bonfireData.push_back(element->BoolAttribute("discovered"));
		}

		node = node->NextSibling();
	}
}

void SaveManager::parseObjectNode(tinyxml2::XMLNode* n, SaveData* data)
{

	tinyxml2::XMLElement* elem = n->ToElement();
	tinyxml2::XMLNode* node = elem->FirstChild();

	if (strcmp(elem->Attribute("name"), "crate") == 0)
	{
		CrateData crateData;

		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				crateData.destRect.x = element->FloatAttribute("x");
				crateData.destRect.y = element->FloatAttribute("y");
				
			}

			if (strcmp(node->Value(), "dimentions") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				crateData.destRect.z = element->FloatAttribute("width");
				crateData.destRect.w = element->FloatAttribute("height");
			}

			if (strcmp(node->Value(), "rotation") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				crateData.angle = element->FloatAttribute("angle");
			}

			node = node->NextSibling();
		}

		data->dynamicCrates.push_back(crateData);
	}

	if (strcmp(elem->Attribute("name"), "skeleton") == 0)
	{
		SkeletonData skeletonData;
		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.position.x = element->FloatAttribute("x");
				skeletonData.position.y = element->FloatAttribute("y");

			}

			if (strcmp(node->Value(), "initialPosition") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.initialPosition.x = element->FloatAttribute("x");
				skeletonData.initialPosition.y = element->FloatAttribute("y");

			}

			node = node->NextSibling();
		}
		data->skeletons.push_back(skeletonData);
	}

	if (strcmp(elem->Attribute("name"), "deadSkeleton") == 0)
	{
		DeadSkeleton skeletonData;

		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.sprite.position.x = element->FloatAttribute("x");
				skeletonData.sprite.position.y = element->FloatAttribute("y");

			}

			if (strcmp(node->Value(), "initialPosition") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.initialPosition.x = element->FloatAttribute("x");
				skeletonData.initialPosition.y = element->FloatAttribute("y");

			}

			if (strcmp(node->Value(), "dimentions") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.sprite.dimentions.x = element->FloatAttribute("width");
				skeletonData.sprite.dimentions.y = element->FloatAttribute("height");
			}

			if (strcmp(node->Value(), "texture") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.sprite.texture = element->Attribute("path");
			}

			node = node->NextSibling();
		}

		data->deadSkeletons.push_back(skeletonData);
	}

	if (strcmp(elem->Attribute("name"), "deadSkeletonBoss") == 0)
	{
		BossSkeletonData skeletonData;

		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.position.x = element->FloatAttribute("x");
				skeletonData.position.y = element->FloatAttribute("y");

			}

			if (strcmp(node->Value(), "name") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				skeletonData.name = element->Attribute("value");

			}

			node = node->NextSibling();
		}

		data->deadSkeletonBosses.push_back(skeletonData);
	}

	if (strcmp(elem->Attribute("name"), "souls") == 0)
	{
		YieldedSoulsData soulsData;

		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				soulsData.pos.x = element->FloatAttribute("x");
				soulsData.pos.y = element->FloatAttribute("y");
			}

			if (strcmp(node->Value(), "amount") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				soulsData.amount = element->Unsigned64Attribute("value");
			}

			node = node->NextSibling();
		}

		data->souls.push_back(soulsData);
	}

	if (strcmp(elem->Attribute("name"), "chest") == 0)
	{
		ChestData chestData;
		while (node)
		{
			if (strcmp(node->Value(), "position") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				chestData.destRect.x = element->FloatAttribute("x");
				chestData.destRect.y = element->FloatAttribute("y");

			}

			if (strcmp(node->Value(), "dimentions") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				chestData.destRect.z = element->FloatAttribute("width");
				chestData.destRect.w = element->FloatAttribute("height");

			}

			if (strcmp(node->Value(), "texture") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();

				if (strcmp(element->Attribute("name"), "opened") == 0)
				{
					chestData.openedTexture = element->Attribute("filename");
				}

				if (strcmp(element->Attribute("name"), "closed") == 0)
				{
					chestData.closedTexture = element->Attribute("filename");
				}
			}

			if (strcmp(node->Value(), "atlas") == 0)
			{
				tinyxml2::XMLElement* element = node->ToElement();
				tinyxml2::XMLNode* atlasNode = element->FirstChild();

				while (atlasNode)
				{
					std::string atlas;
					if (strcmp(atlasNode->Value(), "texture") == 0)
					{
						tinyxml2::XMLElement* textureElement = atlasNode->ToElement();

						if (strcmp(textureElement->Attribute("name"), "opened") == 0)
						{
							chestData.openedAtlasTexture = textureElement->Attribute("filename");
							atlas = "opened";
						}

						if (strcmp(textureElement->Attribute("name"), "opening") == 0)
						{
							chestData.openingAtlasTexture = textureElement->Attribute("filename");
							atlas = "opening";
						}
					}

					if (strcmp(atlasNode->Value(), "properties") == 0)
					{
						tinyxml2::XMLElement* propertiesElement = atlasNode->ToElement();
						tinyxml2::XMLNode* propertiesNode = propertiesElement->FirstChild();

						while (propertiesNode)
						{
							if (strcmp(atlasNode->Value(), "property") == 0)
							{
								tinyxml2::XMLElement* propertyElement = propertiesNode->ToElement();

								if (strcmp(propertyElement->Attribute("name"), "rowsAndCols") == 0)
								{
									if (atlas == "opened")
									{
										chestData.openedAtlasTextureRowsAndCols.x = propertyElement->IntAttribute("x");
										chestData.openedAtlasTextureRowsAndCols.y = propertyElement->IntAttribute("y");
									}

									if (atlas == "opening")
									{
										chestData.openingAtlasTextureRowsAndCols.x = propertyElement->IntAttribute("x");
										chestData.openingAtlasTextureRowsAndCols.y = propertyElement->IntAttribute("y");
									}
								}

								if (strcmp(propertyElement->Attribute("name"), "maxIndex") == 0)
								{
									if (atlas == "opened")
									{
										chestData.openedAtlasTextureMaxIndex = propertyElement->IntAttribute("value");
									}

									if (atlas == "opening")
									{
										chestData.openingAtlasTextureMaxIndex = propertyElement->IntAttribute("value");
									}
								}

								if (strcmp(propertyElement->Attribute("name"), "filtering") == 0)
								{
									chestData.filtering = propertyElement->BoolAttribute("value");
								}
							}

							propertiesNode = propertiesNode->NextSibling();
						}
					}

					atlasNode = atlasNode->NextSibling();
				}
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

							if (strcmp(itemNode->Value(), "destRect") == 0)
							{
								tinyxml2::XMLElement* element = itemNode->ToElement();
								itemData.destRect.x = element->FloatAttribute("x");
								itemData.destRect.y = element->FloatAttribute("y");
								itemData.destRect.z = element->FloatAttribute("width");
								itemData.destRect.w = element->FloatAttribute("height");
							}

							if (strcmp(itemNode->Value(), "description") == 0)
							{
								tinyxml2::XMLElement* element = itemNode->ToElement();

								itemData.description = Engine::String::Trim(element->GetText());
							}

							itemNode = itemNode->NextSibling();
						}

						chestData.items.push_back(itemData);
					}

					inventoryNode = inventoryNode->NextSibling();
				}
			}

			node = node->NextSibling();
		}
		data->chests.push_back(chestData);
	}
}
#include "Chest.h"

Chest::Chest()
{

}

Chest::~Chest()
{

}

void Chest::init(b2World* world, const glm::vec2& position, const glm::vec2& dimentions, const std::string& openingAtlas, const std::string& openedAtlas,
	const std::string& closedTexture, const std::string& openedTexture, std::vector<Item>& items)
{
	m_dimentions = dimentions;
	m_sprite.init(openingAtlas, { position, dimentions }, { 1, 6 }, 5, 1.0, false);
	m_spriteOpened.init(openedAtlas, { position, dimentions }, { 1, 2 }, 1, 1.0, false);

	m_inventory.addItems(items);

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(position.x / PPM, position.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(m_dimentions.x / 2 / PPM, m_dimentions.y / 2 / PPM);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	m_body->CreateFixture(&fixtureDef);

	m_closedTexture = closedTexture;
	m_openedTexture = openedTexture;

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "chest";
	m_body->SetUserData(m_data.get());
}

void Chest::draw()
{
	glm::vec4 destRect = { (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.x / 2, m_dimentions };

	switch (m_state)
	{
	case ChestState::CLOSED:
		Engine::Renderer2D::DrawTexturedQuad(m_closedTexture, destRect, {0.0f, 0.0f, 1.0f, 1.0f}, 1.0f,
			{255, 255, 255, 255}, false);
		break;
	case ChestState::OPENING:
		Engine::Renderer2D::DrawAnimatedQuad(m_sprite);
		break;
	case ChestState::OPENED_FULL:
		Engine::Renderer2D::DrawAnimatedQuad(m_spriteOpened);
		break;
	case ChestState::OPENED_EMPTY:
		Engine::Renderer2D::DrawTexturedQuad(m_openedTexture, destRect, { 0.0f, 0.0f, 1.0f, 1.0f }, 1.0f,
			{ 255, 255, 255, 255 }, false);
		break;
	}
}

ReturnType Chest::update(double deltaTime)
{
	switch (m_state)
	{
	case ChestState::CLOSED:
		
		if (m_data.get()->contact)
		{
			if (m_data.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}

		break;
	case ChestState::OPENING:
		m_sprite.update(deltaTime);
		m_sprite.setPosition({ (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2 });
		if (m_sprite.playAnimation(1.0f / 30.0f))
		{
			m_state = ChestState::OPENED_FULL;
		}
	case ChestState::OPENED_FULL:

		if (m_inventory.isEmpty())
		{
			m_state = ChestState::OPENED_EMPTY;
			break;
		}

		m_spriteOpened.update(deltaTime);
		m_spriteOpened.setPosition({ (m_body->GetPosition().x * PPM) - m_dimentions.x / 2, (m_body->GetPosition().y * PPM) - m_dimentions.y / 2 });
		m_spriteOpened.playAnimation(1.0f / 60.0f);
		
		if (m_data.get()->contact)
		{
			if (m_data.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}


		break;
	case ChestState::OPENED_EMPTY:
		
		if (!m_inventory.isEmpty())
		{
			m_state = ChestState::OPENED_FULL;
		}

		if (m_data.get()->contact)
		{
			if (m_data.get()->with == "player")
			{
				return ReturnType::COLLISION_PLAYER;
			}
		}

		break;
	}

	return ReturnType::NONE;
}

void Chest::addToInventory(Item& item)
{
	m_inventory.addItem(item);
}

void Chest::removeItemFromInventoryById(const std::string& id)
{
	m_inventory.removeById(id);
}

void Chest::removeItemFromInventoryByName(const std::string& name)
{
	m_inventory.removeByName(name);
}

void Chest::removeItemsFromInventoryByName(const std::string& name, uint32_t count)
{
	m_inventory.removeByName(name, count);
}

void Chest::removeAllItemsFromInventoryByName(const std::string& name)
{
	m_inventory.removeAllByName(name);
}

void Chest::removeItems()
{
	m_inventory.removeItems();
}

std::optional<std::vector<Item>> Chest::getItemsByName(const std::string& name)
{
	return m_inventory.getItemsByName(name);
}

std::optional<Item> Chest::getItemById(const std::string& id)
{
	return m_inventory.getItemById(id);
}

std::optional<std::vector<Item>> Chest::getItems()
{
	return m_inventory.getItems();
}
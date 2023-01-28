#include "Player.h"

#include "../Engine/Constants.h"
#include "../Engine/Renderer2D.h"
#include "../Engine/Random.h"

#include "SaveManager.h"

Player::Player()
{

}

Player::~Player()
{

}

void Player::init(b2World* world, const glm::vec2& position, PlayerType type)
{
	m_lastBonfireRestedAtPosition = position;

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x / PPM, position.y / PPM);
	m_body = world->CreateBody(&bodyDef);

	//b2PolygonShape boxShape;
	//boxShape.SetAsBox(PLAYER_WIDTH / 2 / PPM, PLAYER_WIDTH / 2 / PPM);

	b2CircleShape circleShape;
	circleShape.m_radius = PLAYER_WIDTH / 2 / PPM;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 20.0f;
	fixtureDef.friction = 2.0f;
	fixtureDef.filter.groupIndex = -1;
	m_fixture = m_body->CreateFixture(&fixtureDef);

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "player";
	m_body->SetUserData(m_data.get());

	m_type = type;

	b2Vec2 pos = m_body->GetPosition();

	m_idleWatermelonSprite.init("assets/textures/atlases/watermelonAtlas.png", { pos.x * PPM, pos.y * PPM, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 15, 1, true);
	m_deadWatermelonSprite.init("assets/textures/atlases/watermelonDead.png", { pos.x, pos.y, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 15, 1, true);

	//m_idleEarthmelonSprite.init("assets/textures/atlases/earthmelonAtlas.png", { pos.x * PPM, pos.y * PPM, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 4, 1, true);

	m_idleFiremelonSprite.init("assets/textures/atlases/firemelonAtlas.png", { pos.x * PPM, pos.y * PPM, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 4, 1, true);
	m_deadFiremelonSprite.init("assets/textures/atlases/firemelonDead.png", { pos.x, pos.y, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 15, 1, true);

	//m_idleWindSprite.init("assets/textures/atlases/windmelonAtlas.png", { pos.x * PPM, pos.y * PPM, PLAYER_WIDTH, PLAYER_WIDTH }, { 4, 4 }, 4, 1, true);

	m_handSprite.init("assets/textures/atlases/handAtlas.png", { pos.x, pos.y, 32, 32 }, { 1, 4 }, 3, 1, false);
}

void Player::setMovementConfig(Engine::PlayerMovementConfig movementConfig)
{
	m_movementConfig = movementConfig;
}

ReturnType Player::update(b2World* world, Engine::InputManager& inputManager, double deltaTime, GameplayState state)
{
	if (m_attackTimer.ElapsedMillis() >= 100)
	{
		m_canAttack = true;
		m_attackTimer.Reset();
	}

	if (m_staminaTimer.ElapsedMillis() >= 1000)
	{
		m_canStaminaRegen = true;
		m_staminaTimer.Reset();
	}

	if (m_canStaminaRegen && m_playerData.stamina < m_playerData.maxStamina)
	{
		m_playerData.stamina += 100 * deltaTime;
	}

	if (m_attachedToRope)
	{
		m_body->SetAwake(false);
		
		m_counter++;
	}

	if (m_counter >= 30)
	{
		m_counter = 0;
		m_canDettach = true;
	}

	m_playerData.maxHealth = 60 + (glm::pow(glm::log(m_playerData.vigor) * 9.92, 2));
	m_playerData.maxStamina = 60 + (glm::pow(glm::log(m_playerData.endurance) * 7, 2));

	if (m_playerData.health <= 0 && m_animationState != PlayerAnimationState::DEAD)
	{
		m_animationState = PlayerAnimationState::DEAD;
		m_playerData.health = 0;
	}

	if (inputManager.isKeyDown(1) && m_playerData.stamina > 0 && m_restState == RestState::NONE && state != GameplayState::	MENU && m_uiState != UIState::UI)
	{
		m_attackState = PlayerAttackState::REGULAR;
	}
	else
	{
		m_attackState = PlayerAttackState::NONE;
	}

	glm::vec2 pos = glm::vec2((m_body->GetPosition().x * PPM) + PLAYER_WIDTH / 2, m_body->GetPosition().y * PPM);
	if (m_camera->convertScreenToWorld({ inputManager.getMouseCoords().x, 0.0f }).x < pos.x)
	{
		m_handFlip = true;
	}
	else
	{
		m_handFlip = false;
	}

	switch (m_animationState)
	{
	case PlayerAnimationState::WALKING:
		
		if (m_restState == RestState::NONE)
		{
			move(inputManager, deltaTime);
		}

		if (m_restState != RestState::NONE)
		{
			if (inputManager.isKeyPressed(SDLK_ESCAPE))
			{
				m_restState = RestState::NONE;
				leaveBonfire();
			}

			m_body->SetLinearVelocity(b2Vec2(m_body->GetLinearVelocity().x * 0.8f, m_body->GetLinearVelocity().y));
		}

		switch (m_type)
		{
		case PlayerType::WATERMELON:
			m_idleWatermelonSprite.setPosition(glm::vec2((m_body->GetPosition().x * PPM) - PLAYER_WIDTH / 2, (m_body->GetPosition().y * PPM) - PLAYER_WIDTH / 2));
			m_idleWatermelonSprite.update(deltaTime);
			m_idleWatermelonSprite.playAnimation(1.0f / 25.0f);
			break;
		case PlayerType::EARTHMELON:
			break;
		case PlayerType::FIREMELON:
			m_idleFiremelonSprite.setPosition(glm::vec2((m_body->GetPosition().x * PPM) - PLAYER_WIDTH / 2, (m_body->GetPosition().y * PPM) - PLAYER_WIDTH / 2));
			m_idleFiremelonSprite.update(deltaTime);
			m_idleFiremelonSprite.playAnimation(1.0f / 25.0f);
			break;
		case PlayerType::WINDMELON:
			break;
		default:
			break;
		}
		break;
	case PlayerAnimationState::RUNNING:
		break;
	case PlayerAnimationState::IDLE:
		break;
	case PlayerAnimationState::DEAD:
		
		m_body->SetTransform(m_body->GetPosition(), 0.0f);
		m_body->SetLinearVelocity(b2Vec2(m_body->GetLinearVelocity().x * 0.9, m_body->GetLinearVelocity().y));

		switch (m_type)
		{
		case PlayerType::WATERMELON:
			m_deadWatermelonSprite.update(deltaTime);
			m_deadWatermelonSprite.setPosition(glm::vec2((m_body->GetPosition().x * PPM) - PLAYER_WIDTH / 2, (m_body->GetPosition().y * PPM) - PLAYER_WIDTH / 2));

			if (m_deadWatermelonSprite.playAnimation(1.0f / 15.0f))
			{
				// respawn

				m_body->SetTransform({ m_lastBonfireRestedAtPosition.x / PPM, m_lastBonfireRestedAtPosition.y / PPM }, 0.0f);
				m_body->SetAwake(true);
				m_animationState = PlayerAnimationState::WALKING;
				m_playerData.health = m_playerData.maxHealth;
				return ReturnType::DEATH;
			}

			break;
		case PlayerType::EARTHMELON:
			break;
		case PlayerType::FIREMELON:
			m_deadFiremelonSprite.update(deltaTime);
			m_deadFiremelonSprite.setPosition(glm::vec2((m_body->GetPosition().x * PPM) - PLAYER_WIDTH / 2, (m_body->GetPosition().y * PPM) - PLAYER_WIDTH / 2));

			if (m_deadFiremelonSprite.playAnimation(1.0f / 15.0f))
			{
				// respawn

				m_body->SetTransform({ m_lastBonfireRestedAtPosition.x / PPM, m_lastBonfireRestedAtPosition.y / PPM }, 0.0f);
				m_body->SetAwake(true);
				m_animationState = PlayerAnimationState::WALKING;
				m_playerData.health = m_playerData.maxHealth;
				return ReturnType::DEATH;
			}

			break;
		case PlayerType::WINDMELON:
			break;
		default:
			break;
		}

		return ReturnType::DYING;

		break;
	default:
		break;
	}

	if (m_animationState != PlayerAnimationState::DEAD)
	{
		switch (m_attackState)
		{
		case PlayerAttackState::NONE:

			break;
		case PlayerAttackState::REGULAR:

			glm::vec2 pos = glm::vec2((m_body->GetPosition().x * PPM) + PLAYER_WIDTH / 2, m_body->GetPosition().y * PPM);

			if (m_handFlip)
			{
				pos.x = (m_body->GetPosition().x * PPM) - (PLAYER_WIDTH / 2) - 32;
			}

			m_handSprite.update(deltaTime);
			m_handSprite.setPosition(pos);
			if (m_handSprite.playAnimation(1.0f / 2.0f))
			{
				if (inputManager.isKeyDown(1) && m_canAttack)
				{
					shootProjectile(world, inputManager, { 32, 32 }, { 10, 10 }, { 7.5f, 7.5f }, 
						(m_handFlip) ? glm::vec2((-PLAYER_WIDTH / 2) - 32, 10.0f) : glm::vec2((PLAYER_WIDTH / 2) + 32, 10.0f));

					m_playerData.stamina -= 5.0f / (m_playerData.endurance * 0.05);

					m_canAttack = false;
					m_canStaminaRegen = false;
					m_staminaTimer.Reset();
				}
			}
			break;
		default:
			break;
		}
	}

	if (m_playerData.stamina <= 0)
	{
		m_playerData.stamina = 0;
	}

	if (m_playerData.health <= 0)
	{
		m_playerData.health = 0;
	}

	return ReturnType::NONE;
}

void Player::draw(Engine::ParticleSystem2D& particleSystem)
{

	switch (m_animationState)
	{
	case PlayerAnimationState::WALKING:
		
		switch (m_type)
		{
		case PlayerType::WATERMELON:
			Engine::Renderer2D::DrawAnimatedQuad(m_idleWatermelonSprite, m_body->GetAngle());
			break;
		case PlayerType::EARTHMELON:
			break;
		case PlayerType::FIREMELON:
			Engine::Renderer2D::DrawAnimatedQuad(m_idleFiremelonSprite, m_body->GetAngle());
			break;
		case PlayerType::WINDMELON:
			break;
		default:
			break;
		}

		break;
	case PlayerAnimationState::RUNNING:
		break;
	case PlayerAnimationState::IDLE:
		break;
	case PlayerAnimationState::DEAD:

		switch (m_type)
		{
		case PlayerType::WATERMELON:
			Engine::Renderer2D::DrawAnimatedQuad(m_deadWatermelonSprite, m_body->GetAngle());
			break;
		case PlayerType::EARTHMELON:
			break;
		case PlayerType::FIREMELON:
			Engine::Renderer2D::DrawAnimatedQuad(m_deadFiremelonSprite, m_body->GetAngle());
			break;
		case PlayerType::WINDMELON:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	if (m_animationState != PlayerAnimationState::DEAD)
	{
		switch (m_attackState)
		{
		case PlayerAttackState::NONE:

			break;
		case PlayerAttackState::REGULAR:
			Engine::Renderer2D::DrawAnimatedQuad(m_handSprite, (m_handFlip) ? Engine::FlipType::FLIP_X : Engine::FlipType::NONE);
			break;
		default:
			break;
		}
	}
}

void Player::move(Engine::InputManager& inputManager, double deltaTime)
{
	if (inputManager.isKeyDown(SDLK_a) || inputManager.isKeyDown(SDLK_LEFT))
	{
		m_body->ApplyForceToCenter(b2Vec2(-100.0f, 0), true);
	}
	else if (inputManager.isKeyDown(SDLK_d) || inputManager.isKeyDown(SDLK_RIGHT))
	{
		m_body->ApplyForceToCenter(b2Vec2(100.0f, 0), true);
	}
	else
	{
		m_body->SetLinearVelocity(b2Vec2(m_body->GetLinearVelocity().x * 0.9, m_body->GetLinearVelocity().y));
	}

	if (m_body->GetLinearVelocity().x < -MAX_SPEED)
	{
		m_body->SetLinearVelocity(b2Vec2(-MAX_SPEED, m_body->GetLinearVelocity().y));
	}
	else if (m_body->GetLinearVelocity().x > MAX_SPEED) 
	{
		m_body->SetLinearVelocity(b2Vec2(MAX_SPEED, m_body->GetLinearVelocity().y));
	}

	// Loop through all the contact points
	for (b2ContactEdge* ce = m_body->GetContactList(); ce != nullptr; ce = ce->next) 
	{
		b2Contact* c = ce->contact;
		if (c->IsTouching()) 
		{
			b2WorldManifold manifold;
			c->GetWorldManifold(&manifold);
			// Check if the points are below
			bool below = false;
			for (int i = 0; i < b2_maxManifoldPoints; i++) 
			{
				if (manifold.points[i].y < m_body->GetPosition().y - PLAYER_WIDTH / PPM / 2.0f + 0.01f)
				{
					below = true;
					break;
				}
			}

			if (below)
			{
				// We can jump
				if (inputManager.isKeyPressed(SDLK_SPACE) || inputManager.isKeyPressed(SDLK_w) || inputManager.isKeyPressed(SDLK_UP))
				{
					if (!m_attachedToRope)
					{
						m_body->ApplyLinearImpulse(b2Vec2(0.0f, 25.0f), m_body->GetWorldCenter(), true);
						m_playerData.stamina -= 50.0f / (m_playerData.endurance * 0.1);
						m_canStaminaRegen = false;
						m_staminaTimer.Reset();
					}
					break;
				}
			}
		}
	}
}

void Player::restAtBonfire(Bonfire bonfire)
{
	m_lastBonfireRestedAtPosition = bonfire.getPosition();
	m_playerData.health = m_playerData.maxHealth;
	m_playerData.stamina = m_playerData.maxStamina;
	m_restState = RestState::SITTING;
	m_isAtBonfire = true;
}

void Player::shootProjectile(b2World* world, Engine::InputManager& inputManager, const glm::vec2& size, const glm::vec2& colliderSize, const glm::vec2& power, glm::vec2 offset)
{
	glm::vec2 destination = inputManager.getMouseCoords();
	destination = m_camera->convertScreenToWorld(destination);

	glm::vec2 pos = { (m_body->GetPosition().x * PPM) + offset.x, (m_body->GetPosition().y * PPM) + offset.y };

	glm::vec2 direction = destination - pos;
	direction = glm::normalize(direction);

	const glm::vec2 right(1.0f, 0.0f);
	float angle = acos(glm::dot(right, direction));
	if (direction.y < 0.0f)
	{
		angle = -angle;
	}

	Projectile projectile;
	projectile.init(world, pos, size, colliderSize, b2Vec2(power.x * direction.x, power.y * direction.y), angle, "playerProjectile", 1);
	m_projectiles->push_back(projectile);
}

void Player::attachToRope(uint32_t id)
{
	m_attachedToRope = true;
	m_ropeId = id;
}

void Player::dettachFromRope(const b2Vec2& velocity)
{
	m_body->SetLinearVelocity({ velocity.x, velocity.y * 1.1f });

	m_attachedToRope = false;
	m_ropeId = 0;
	m_canDettach = false;
}

bool Player::isAtBonfire()
{
	return m_isAtBonfire;
}

void Player::addToInventory(Item& item)
{
	m_inventory.addItem(item);
}

void Player::removeItemFromInventoryById(const std::string& id)
{
	m_inventory.removeById(id);
}

void Player::removeItemFromInventoryByName(const std::string& name)
{
	m_inventory.removeByName(name);
}

void Player::removeItemsFromInventoryByName(const std::string& name, uint32_t count)
{
	m_inventory.removeByName(name, count);
}

void Player::removeAllItemsFromInventoryByName(const std::string& name)
{
	m_inventory.removeAllByName(name);
}

void Player::removeItems()
{
	m_inventory.removeItems();
}

std::optional<std::vector<Item>> Player::getItemsByName(const std::string& name)
{
	return m_inventory.getItemsByName(name);
}

std::optional<Item> Player::getItemById(const std::string& id)
{
	return m_inventory.getItemById(id);
}

std::optional<std::vector<Item>> Player::getItems()
{
	return m_inventory.getItems();
}
#include "Skeleton.h"

#include "../Engine/PathFinding.h"
#include "../Engine/Random.h"

Skeleton::Skeleton()
{

}

Skeleton::~Skeleton()
{
	
}

void Skeleton::init(b2World* world, const glm::vec2& position)
{
	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "skeleton";

	m_dimentions = { 200, 200 };
	m_experienceYield = 87;
	m_health = 350;
	m_maxHealth = 350;


	m_capsuleCollider.init(world, position, { 75, 150 }, 10.0f, 0.3f, true);

	auto body = m_capsuleCollider.getBody();
	body->SetUserData(m_data.get());

	glm::vec4 destRect = { body->GetPosition().x * PPM, body->GetPosition().y * PPM, m_dimentions };
	m_initialPosition = { destRect.x, destRect.y };
	m_sprite.init("assets/textures/enemies/skeleton/atlases/skeletonWalk.png", destRect, { 1, 9 }, 8, 1.0, false);
	m_attackSprite.init("assets/textures/enemies/skeleton/atlases/skeletonAttack.png", destRect, { 1, 13 }, 12, 1.0, false);
	m_deathSprite.init("assets/textures/enemies/skeleton/atlases/skeletonDeath.png", destRect, { 1, 6 }, 5, 1.0, false);
	m_idleSprite.init("assets/textures/enemies/skeleton/atlases/skeletonIdle.png", destRect, { 1, 7 }, 6, 1.0, false);
}

void Skeleton::onInit()
{
	
}

ReturnType Skeleton::update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager)
{
	if (m_timer.ElapsedMillis() >= 2000)
	{
		m_canAttack = true;
		m_timer.Reset();
	}

	if (m_jumpTimer.ElapsedMillis() >= 5000)
	{
		m_canJump = true;
		m_jumpTimer.Reset();
	}

	if (m_audioTimer.ElapsedMillis() >= Engine::Random::randDouble(1.5, 2.5) * 1000.0)
	{
		m_canPlayAudio = true;
		m_audioTimer.Reset();
	}

	move(deltaTime, { 7.5f , 3.0f }, { 2.5f, 1.5f }, 50.0f, 20.0f);
	attack();
	takeDamage();

	auto body = m_capsuleCollider.getBody();

	switch (m_animationState)
	{
	case AnimationState::DEATH:
		m_deathSprite.update(deltaTime);
		m_deathSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		if (m_deathSprite.playAnimation(1.0f / 4.0f))
		{
			return ReturnType::DEATH;
		}
		break;
	case AnimationState::ATTACK:
		m_attackSprite.update(deltaTime);
		m_attackSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		if (m_attackSprite.playAnimation(1.0f / 4.0f))
		{
			m_animationState = AnimationState::WALK;

			// shoot projectile

			shootProjectile(world, { 64, 64 }, { 1, 1 }, { 10.0f, 10.0f }, glm::vec2(30, 10));

		}
	
	case AnimationState::WALK:
		m_sprite.update(deltaTime);
		m_sprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		m_sprite.playAnimation(1.0f / 4.0f);

		if (m_canPlayAudio)
		{
			uint32_t random = Engine::Random::randInt(1, 4);

			audioEngine.playSound("assets/audio/enemies/skeleton/skeleton_footstep_" + std::to_string(random) + ".mp3",
				{ m_capsuleCollider.getBody()->GetPosition().x, m_capsuleCollider.getBody()->GetPosition().y, 0.0f });

			m_canPlayAudio = false;
		}
		
	case AnimationState::IDLE:
		m_idleSprite.update(deltaTime);
		m_idleSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		m_idleSprite.playAnimation(1.0f / 4.0f);
		
	}

	if (m_data.get()->contact)
	{
		if (m_data.get()->with == "playerProjectile")
		{
			return ReturnType::COLLISION_PLAYER_PROJECTILE;
		}
	}

	return ReturnType::NONE;
}

void Skeleton::draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims)
{
	auto body = m_capsuleCollider.getBody();

	glm::vec2 skeletonPosition = { body->GetPosition().x, body->GetPosition().y };

	switch (m_animationState)
	{
	case AnimationState::ATTACK:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_attackSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_attackSprite);
		}
		break;
	case AnimationState::WALK:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_sprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_sprite);
		}
		break;
	case AnimationState::DEATH:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_deathSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_deathSprite);
		}
		break;
	case AnimationState::IDLE:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_idleSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			Engine::Renderer2D::DrawAnimatedQuad(m_idleSprite);
		}
		break;
	}

	if (m_animationState != AnimationState::DEATH)
	{
		drawHealthBar(camera, screenDims);
	}
}

void Skeleton::move(double deltaTime, const glm::vec2& aggroPadding, const glm::vec2& attackPadding, float velocity, float jumpPower)
{
	// AI

	m_playerPosition = m_player->getPosition() / PPM;
	auto body = m_capsuleCollider.getBody();

	glm::vec2 skeletonPosition = { body->GetPosition().x, body->GetPosition().y };
	if (skeletonPosition.x > m_playerPosition.x - aggroPadding.x && skeletonPosition.x < m_playerPosition.x + aggroPadding.x
		&& skeletonPosition.y > m_playerPosition.y - aggroPadding.y && skeletonPosition.y < m_playerPosition.y + aggroPadding.y)
	{
		m_isPlayerInAgrroRange = true;
	}
	else
	{
		m_isPlayerInAgrroRange = false;
	}

	if (m_isPlayerInAgrroRange && m_player->getAnimationState() != PlayerAnimationState::DEAD && m_player->getRestState() == RestState::NONE)
	{
		if (skeletonPosition.x < m_playerPosition.x)
		{
			body->ApplyForceToCenter(b2Vec2(velocity, 0.0f), true);
			if (m_animationState != AnimationState::ATTACK)
			{
				m_animationState = AnimationState::WALK;
			}
		}

		if (skeletonPosition.x > m_playerPosition.x)
		{
			body->ApplyForceToCenter(b2Vec2(-velocity, 0.0f), true);
			if (m_animationState != AnimationState::ATTACK)
			{
				m_animationState = AnimationState::WALK;
			}
		}

		if (skeletonPosition.y < m_playerPosition.y - 0.2 && canJump())
		{
			body->ApplyLinearImpulse(b2Vec2(0.0f, jumpPower), body->GetWorldPoint(b2Vec2(0.0f, 0.0f)), true);
			m_canJump = false;
		}
	}
	else
	{
		body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x * 0.9, body->GetLinearVelocity().y));
	}

	if (glm::abs(body->GetLinearVelocity().x) < 0.1f)
	{
		if (m_animationState != AnimationState::ATTACK)
		{
			m_animationState = AnimationState::IDLE;
		}
	}

	if (body->GetLinearVelocity().x < -m_maxSpeed)
	{
		body->SetLinearVelocity(b2Vec2(-m_maxSpeed, body->GetLinearVelocity().y));
	}
	else if (body->GetLinearVelocity().x > m_maxSpeed)
	{
		body->SetLinearVelocity(b2Vec2(m_maxSpeed, body->GetLinearVelocity().y));
	}
	
	glm::vec2 skeletonPositionAttack = { body->GetPosition().x, body->GetPosition().y };
	if (skeletonPositionAttack.x > m_playerPosition.x - attackPadding.x && skeletonPositionAttack.x < m_playerPosition.x + attackPadding.x
		&& skeletonPosition.y > m_playerPosition.y - attackPadding.y && skeletonPosition.y < m_playerPosition.y + attackPadding.y)
	{
		m_isPlayerClose = true;
	}
	else
	{
		m_isPlayerClose = false;
	}

	// TODO: implement monte carlo simulation


}

void Skeleton::attack()
{
	
	if (m_canAttack && m_isPlayerClose && m_player->getAnimationState() != PlayerAnimationState::DEAD && m_player->getRestState() == RestState::NONE)
	{
		m_animationState = AnimationState::ATTACK;
		m_canAttack = false;
	}

}

void Skeleton::takeDamage()
{
	//m_health--;

	if (m_health < 1)
	{
		m_animationState = AnimationState::DEATH;
	}
}

void Skeleton::collectPlayerData(Player* player)
{
	m_player = player;
}

void Skeleton::collectProjectileData(std::vector<Projectile>* projectiles)
{
	m_projectiles = projectiles;
}

void Skeleton::collectGridData(const std::vector<std::vector<int>>& grid)
{
	m_grid = grid;
}

bool Skeleton::canJump()
{
	auto body = m_capsuleCollider.getBody();
	for (b2ContactEdge* ce = body->GetContactList(); ce != nullptr; ce = ce->next)
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
				if (manifold.points[i].y < body->GetPosition().y - m_dimentions.y / PPM / 2.0f + 0.01f)
				{
					below = true;
					return below && m_canJump;
				}
			}
		}
	}
}

void Skeleton::shootProjectile(b2World* world, const glm::vec2& size, const glm::vec2& colliderSize, const glm::vec2& power, glm::vec2 offset)
{
	glm::vec2 playerPos = m_player->getPosition();
	auto body = m_capsuleCollider.getBody();

	if (m_playerPosition.x < body->GetPosition().x * PPM)
		offset = glm::vec2(-offset.x, offset.y);

	glm::vec2 pos = { (body->GetPosition().x * PPM) + offset.x, (body->GetPosition().y * PPM) + offset.y };

	glm::vec2 direction = playerPos - pos;
	direction = glm::normalize(direction);

	glm::vec2 vA(pos.y - playerPos.y, pos.x - playerPos.x);

	float distance = glm::sqrt(glm::pow(vA.x, 2) + glm::pow(vA.y, 2));

	const glm::vec2 right(1.0f, 0.0f);
	float angle = acos(glm::dot(right, direction));
	if (direction.y < 0.0f)
	{
		angle = -angle;
	}

	Projectile projectile;
	projectile.init(world, pos, size, colliderSize, b2Vec2(power.x * direction.x, (power.y * direction.y) + 1.0f), angle, "projectile", -1);
	m_projectiles->push_back(projectile);
}

void Skeleton::drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims)
{
	glm::vec2 pos = { m_capsuleCollider.getBody()->GetPosition().x * PPM, m_capsuleCollider.getBody()->GetPosition().y * PPM };
	glm::vec2 healthBarPos;
	if (camera.isBoxInView(pos, m_dimentions))
	{
		if (pos.y + (m_dimentions.y / 2) > camera.getPosition().y + (screenDims.y / 2) - 100)
		{
			healthBarPos = glm::vec2((m_capsuleCollider.getBody()->GetPosition().x * PPM) - m_dimentions.x / 2, camera.getPosition().y + (screenDims.y / 2) - 80);
		}
		else if (pos.y + (m_dimentions.y / 2) < camera.getPosition().y - (screenDims.y / 2) + 100)
		{
			healthBarPos = glm::vec2((m_capsuleCollider.getBody()->GetPosition().x * PPM) - m_dimentions.x / 2, camera.getPosition().y - (screenDims.y / 2) + 80);
		}
		else
		{
			healthBarPos = glm::vec2((m_capsuleCollider.getBody()->GetPosition().x * PPM) - m_dimentions.x / 2, ((float)m_capsuleCollider.getBody()->GetPosition().y * PPM) + m_dimentions.x / 1.5);
		}
	}
	else
	{
		healthBarPos = glm::vec2((m_capsuleCollider.getBody()->GetPosition().x * PPM) - m_dimentions.x / 2, ((float)m_capsuleCollider.getBody()->GetPosition().y * PPM) + m_dimentions.x / 1.5);
	}


	// health bar
	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_background.png",
		{ healthBarPos.x + 10, healthBarPos.y, 200, 16 },
		{ 0.0f, 0.0f, 1.0f, 1.0f });

	float x = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
	if (x <= 0.0f) x = 0.0f;

	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar.png",
		{ healthBarPos.x + 10, healthBarPos.y, 200.0f * (static_cast<float>(m_health) / static_cast<float>(m_maxHealth)), 16 },
		{ 0.0f, 0.0f, x, 1.0f });

	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_outline_enemy.png", { healthBarPos.x, healthBarPos.y - 7, 220, 30 },
		{ 0.0f, 0.0f, 1.0f, 1.0f });
}
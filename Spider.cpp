#include "Spider.h"

Spider::Spider()
{

}

Spider::~Spider()
{

}

void Spider::init(b2World* world, const glm::vec2& position)
{
	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "spider";

	m_dimentions = { 200, 200 };
	m_experienceYield = 102;
	m_health = 450;
	m_maxHealth = 450;

	m_capsuleCollider.init(world, position, { 100, 100 }, 10.0f, 0.3f, true);

	auto body = m_capsuleCollider.getBody();
	body->SetUserData(m_data.get());

	glm::vec4 destRect = { body->GetPosition().x * PPM, body->GetPosition().y * PPM, m_dimentions };
	m_initialPosition = { destRect.x, destRect.y };

	m_sprite.init("assets/textures/enemies/spider/atlases/spiderWalk.png", destRect, { 1, 9 }, 8, 1.0, false);
}

void Spider::onInit()
{

}

ReturnType Spider::update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager)
{
	move(deltaTime, { 7.5f , 3.0f }, { 2.5f, 1.5f }, 50.0f, 20.0f);
	attack();
	takeDamage();

	auto body = m_capsuleCollider.getBody();

	switch (m_animationState)
	{
	case AnimationState::DEATH:
		//m_deathSprite.update(deltaTime);
		//m_deathSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		//if (m_deathSprite.playAnimation(1.0f / 4.0f))
		//{
			return ReturnType::DEATH;
		//}
		break;
	case AnimationState::ATTACK:
		//m_attackSprite.update(deltaTime);
		//m_attackSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		//if (m_attackSprite.playAnimation(1.0f / 4.0f))
		//{
		//	m_animationState = AnimationState::WALK;

			// shoot web

			//shootProjectile(world, { 64, 64 }, { 1, 1 }, { 10.0f, 10.0f }, glm::vec2(30, 10));



		//}

	case AnimationState::WALK:
		m_sprite.update(deltaTime);
		m_sprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		m_sprite.playAnimation(1.0f / 4.0f);

		/*if (m_canPlayAudio)
		{
			uint32_t random = Engine::Random::randInt(1, 4);

			audioEngine.playSound("assets/audio/enemies/skeleton/skeleton_footstep_" + std::to_string(random) + ".mp3",
				{ m_capsuleCollider.getBody()->GetPosition().x, m_capsuleCollider.getBody()->GetPosition().y, 0.0f });

			m_canPlayAudio = false;
		}*/

	case AnimationState::IDLE:
		//m_idleSprite.update(deltaTime);
		//m_idleSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - m_capsuleCollider.getDimensions().y / 2));
		//m_idleSprite.playAnimation(1.0f / 4.0f);
		break;
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

void Spider::draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims)
{
	auto body = m_capsuleCollider.getBody();

	glm::vec2 skeletonPosition = { body->GetPosition().x, body->GetPosition().y };

	switch (m_animationState)
	{
	case AnimationState::ATTACK:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			//Engine::Renderer2D::DrawAnimatedQuad(m_attackSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			//Engine::Renderer2D::DrawAnimatedQuad(m_attackSprite);
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
			//Engine::Renderer2D::DrawAnimatedQuad(m_deathSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			//Engine::Renderer2D::DrawAnimatedQuad(m_deathSprite);
		}
		break;
	case AnimationState::IDLE:
		if (skeletonPosition.x > m_playerPosition.x)
		{
			//Engine::Renderer2D::DrawAnimatedQuad(m_idleSprite, Engine::FlipType::FLIP_X);
		}
		else
		{
			//Engine::Renderer2D::DrawAnimatedQuad(m_idleSprite);
		}
		break;
	}

	if (m_animationState != AnimationState::DEATH)
	{
		drawHealthBar(camera, screenDims);
	}
}

void Spider::move(double deltaTime, const glm::vec2& aggroPadding, const glm::vec2& attackPadding, float velocity, float jumpPower)
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

void Spider::attack()
{

	if (m_canAttack && m_isPlayerClose && m_player->getAnimationState() != PlayerAnimationState::DEAD && m_player->getRestState() == RestState::NONE)
	{
		m_animationState = AnimationState::ATTACK;
		m_canAttack = false;
	}

}

void Spider::takeDamage()
{
	//m_health--;

	if (m_health < 1)
	{
		m_animationState = AnimationState::DEATH;
	}
}

void Spider::drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims)
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
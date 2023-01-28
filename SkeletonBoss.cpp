#include "SkeletonBoss.h"

SkeletonBoss::SkeletonBoss()
{

}

SkeletonBoss::~SkeletonBoss()
{

}

void SkeletonBoss::init(b2World* world, const glm::vec2 pos)
{
	m_maxSpeed = 3.0f;
	m_health = 3500;
	m_maxHealth = 3500;
	m_experienceYield = 3537;

	m_dimentions = { 600, 600 };

	m_capsuleCollider.init(world, pos, { 225, 450 }, 1.0f, 0.3f, true);

	m_data = std::make_shared<EntityData>();
	m_data.get()->type = "skeletonBoss";
	m_capsuleCollider.getBody()->SetUserData(m_data.get());

	auto body = m_capsuleCollider.getBody();

	glm::vec4 destRect = { body->GetPosition().x * PPM, body->GetPosition().y * PPM, m_dimentions };

	m_sprite.init("assets/textures/enemies/skeleton/atlases/skeletonWalk.png", destRect, { 1, 9 }, 8, 1.0, false);
	m_attackSprite.init("assets/textures/enemies/skeleton/atlases/skeletonAttack.png", destRect, { 1, 13 }, 12, 1.0, false);
	m_deathSprite.init("assets/textures/enemies/skeleton/atlases/skeletonDeath.png", destRect, { 1, 6 }, 5, 1.0, false);
	m_idleSprite.init("assets/textures/enemies/skeleton/atlases/skeletonIdle.png", destRect, { 1, 7 }, 6, 1.0, false);
}

void SkeletonBoss::onInit()
{

}

ReturnType SkeletonBoss::update(double deltaTime, b2World* world, Engine::AudioEngine& audioEngine, Engine::InputManager& inputManager)
{
	if (m_timer.ElapsedMillis() >= 1000)
	{
		m_canAttack = true;
		m_timer.Reset();
	}

	if (m_jumpTimer.ElapsedMillis() >= 2000)
	{
		m_canJump = true;
		m_jumpTimer.Reset();
	}

	move(deltaTime, { 15.0f , 15.0f }, { 5.0f, 2.5f }, 150.0f, 30.0f);
	attack();
	takeDamage();

	auto body = m_capsuleCollider.getBody();
	float yOffset = m_capsuleCollider.getDimensions().y / 2;

	switch (m_animationState)
	{
	case AnimationState::DEATH:
		m_deathSprite.update(deltaTime);
		m_deathSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - yOffset));
		if (m_deathSprite.playAnimation(1.0f / 4.0f))
		{
			return ReturnType::DEATH;
		}
		break;
	case AnimationState::ATTACK:
		m_attackSprite.update(deltaTime);
		m_attackSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - yOffset));
		if (m_attackSprite.playAnimation(1.0f / 2.0f))
		{
			m_animationState = AnimationState::WALK;

			shootProjectile(world, { 128, 128 }, { 10, 10 }, { 20.0f, 20.0f }, { 0, 200 });
		}
	case AnimationState::WALK:
		m_sprite.update(deltaTime);
		m_sprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - yOffset));
		m_sprite.playAnimation(1.0f / 4.0f);

	case AnimationState::IDLE:
		m_idleSprite.update(deltaTime);
		m_idleSprite.setPosition(glm::vec2((body->GetPosition().x * PPM) - m_dimentions.x / 2, (body->GetPosition().y * PPM) - yOffset));
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

void SkeletonBoss::draw(Engine::ParticleSystem2D& particleSystem, Engine::Camera& camera, const glm::vec2& screenDims)
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

	if (m_animationState != AnimationState::DEATH && m_drawHealthBar)
	{
		drawHealthBar(camera, screenDims);
	}
}

void SkeletonBoss::drawHealthBar(Engine::Camera& camera, const glm::vec2& screenDims)
{
	float ratio = screenDims.y / 1080.0f;
	float padding = 400.0f * ratio;
	float width = (1920.0f * ratio);
	glm::vec2 camPos = camera.getPosition();
	glm::vec2 healthBarPos = glm::vec2(camPos.x - (screenDims.x / 2) + padding / 2.0f, camPos.y - (screenDims.y / 2) + padding / 2.0f);

	// health bar
	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_background.png",
		{ healthBarPos.x + (screenDims.x / 2) - (width / 2), healthBarPos.y, width - padding, 16 },
		{ 0.0f, 0.0f, 1.0f, 1.0f });

	float x = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
	if (x <= 0.0f) x = 0.0f;

	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar.png",
		{ healthBarPos.x + (screenDims.x / 2) - (width / 2), healthBarPos.y, (width - padding) * (static_cast<float>(m_health) / static_cast<float>(m_maxHealth)), 16 },
		{ 0.0f, 0.0f, x, 1.0f });

	Engine::Renderer2D::DrawTexturedQuad("ui/health_bar_outline.png", { healthBarPos.x + (screenDims.x / 2) - (width / 2), healthBarPos.y - 7, width - padding, 30 },
		{ 0.0f, 0.0f, 1.0f, 1.0f });

	Engine::Renderer2D::DrawText("Lato-Regular.ttf", m_name, { healthBarPos.x + (screenDims.x / 2) - (width / 2), healthBarPos.y + 24 }, 20);
}
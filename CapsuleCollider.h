#pragma once


#include <Box2D/Box2D.h>
#include <glm.hpp>


class CapsuleCollider
{
public:
    void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, float density, float friction, bool fixedRotation)
    {
        m_dimensions = dimensions;

        // Make the body
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(position.x / PPM, position.y / PPM);
        bodyDef.fixedRotation = fixedRotation;
        m_body = world->CreateBody(&bodyDef);

        // Create the box
        b2PolygonShape boxShape;
        boxShape.SetAsBox(dimensions.x / PPM / 2.0f, dimensions.y / PPM / 4.0f);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = density;
        fixtureDef.friction = friction;
        m_fixtures[0] = m_body->CreateFixture(&fixtureDef);

        // Create the circles
        b2CircleShape circleShape;
        circleShape.m_radius = dimensions.x / PPM / 2.0f;

        b2FixtureDef circleDef;
        circleDef.shape = &circleShape;
        circleDef.density = density;
        circleDef.friction = friction;

        // Top circle
        circleShape.m_p.Set(0.0f, (-m_dimensions.y + dimensions.x) / PPM / 2.0f);
        m_fixtures[1] = m_body->CreateFixture(&circleDef);

        // Bottom Circle
        circleShape.m_p.Set(0.0f, (m_dimensions.y - dimensions.x) / PPM / 2.0f);
        m_fixtures[2] = m_body->CreateFixture(&circleDef);
    }

    b2Body* getBody() const { return m_body; }
    b2Fixture* getFixture(uint32_t index) const { return m_fixtures[index]; }
    const glm::vec2& getDimensions() const { return m_dimensions; }

private:
    b2Body* m_body = nullptr;
    b2Fixture* m_fixtures[3];
    glm::vec2 m_dimensions;
};
#pragma once

#include <Box2D/Box2D.h>

#include "Bonfire.h"
#include "Player.h"
#include "Projectile.h"
#include "Chest.h"

#include "EntityData.h"


class ContactListener : public b2ContactListener
{
public:
	ContactListener() {}
	~ContactListener() {}

	virtual void BeginContact(b2Contact* contact)
	{

		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		EntityData* fixtureAData = static_cast<EntityData*>(fixtureA->GetBody()->GetUserData());
		EntityData* fixtureBData = static_cast<EntityData*>(fixtureB->GetBody()->GetUserData());

		if (fixtureAData && fixtureBData)
		{
			if (fixtureAData->type == "bonfire" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "chest" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "crate" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "skeleton" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "projectile" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "skeleton")
			{
				//printf("jd\n");
				fixtureBData->contact = true;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = true;
				fixtureAData->with = "skeleton";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "spider")
			{
				//printf("jd\n");
				fixtureBData->contact = true;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = true;
				fixtureAData->with = "spider";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "skeletonBoss")
			{
				//printf("jd\n");
				fixtureBData->contact = true;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = true;
				fixtureAData->with = "skeletonBoss";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "spikes" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "npc" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "item")
			{
				fixtureBData->contact = true;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "item" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "souls")
			{
				fixtureBData->contact = true;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "souls" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "rope")
			{
				fixtureBData->contact = true;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "rope" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "trigger" && fixtureBData->type == "player")
			{
				fixtureAData->contact = true;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}
		}
	}

	virtual void EndContact(b2Contact* contact)
	{
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		EntityData* fixtureAData = static_cast<EntityData*>(fixtureA->GetBody()->GetUserData());
		EntityData* fixtureBData = static_cast<EntityData*>(fixtureB->GetBody()->GetUserData());

		if (fixtureAData && fixtureBData)
		{
			if (fixtureAData->type == "bonfire" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "chest" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "crate" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "skeleton" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "projectile" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "skeleton")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = false;
				fixtureAData->with = "skeleton";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "spider")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = false;
				fixtureAData->with = "spider";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "playerProjectile" && fixtureBData->type == "skeletonBoss")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "playerProjectile";

				fixtureB->GetBody()->SetUserData(fixtureBData);

				fixtureAData->contact = false;
				fixtureAData->with = "skeletonBoss";

				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "spikes" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "npc" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "item")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "item" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "souls")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "souls" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "player" && fixtureBData->type == "rope")
			{
				fixtureBData->contact = false;
				fixtureBData->with = "player";
				fixtureB->GetBody()->SetUserData(fixtureBData);
			}

			if (fixtureAData->type == "rope" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}

			if (fixtureAData->type == "trigger" && fixtureBData->type == "player")
			{
				fixtureAData->contact = false;
				fixtureAData->with = "player";
				fixtureA->GetBody()->SetUserData(fixtureAData);
			}
		}
	}

	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{

	}

	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{

	}

};

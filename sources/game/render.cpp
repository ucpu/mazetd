#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	void gameReset()
	{
		engineEntities()->destroy();
	}

	void gameUpdate()
	{
		entitiesVisitor([](Entity *e, MovementComponent &mv, EngineComponent &ec) {
			TransformComponent &t = ec.entity->value<TransformComponent>();
			const Real f = mv.timeEnd > mv.timeStart ? saturate(Real(gameTime - mv.timeStart) / (mv.timeEnd - mv.timeStart)) : 0;
			const Vec3 a = globalGrid->center(mv.tileStart);
			const Vec3 b = globalGrid->center(mv.tileEnd);
			t.position = interpolate(a, b, f);
			t.orientation = Quat(b - a, Vec3(0, 1, 0));
		}, gameEntities(), false);
	}

	void engineComponentAdded(Entity *e)
	{
		Entity *f = e->value<EngineComponent>().entity = engineEntities()->createAnonymous();
		EntityComponent *pc = gameEntities()->component<PositionComponent>();
		if (e->has(pc))
		{
			TransformComponent &t = f->value<TransformComponent>();
			PositionComponent &p = e->value<PositionComponent>(pc);
			t.position = globalGrid->center(p.tile);
		}
	}

	void engineComponentRemoved(Entity *e)
	{
		Entity *f = e->value<EngineComponent>().entity;
		if (f)
			f->destroy();
	}

	struct Callbacks
	{
		EventListener<void()> gameResetListener;
		EventListener<void()> gameUpdateListener;
		EventListener<void(Entity *)> engineComponentAddedListener;
		EventListener<void(Entity *)> engineComponentRemovedListener;

		Callbacks()
		{
			gameResetListener.attach(eventGameReset(), -90);
			gameResetListener.bind<&gameReset>();
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
			{
				EntityComponent *ec = gameEntities()->component<EngineComponent>();
				engineComponentAddedListener.attach(ec->group()->entityAdded);
				engineComponentAddedListener.bind<&engineComponentAdded>();
				engineComponentRemovedListener.attach(ec->group()->entityRemoved);
				engineComponentRemovedListener.bind<&engineComponentRemoved>();
			}
		}
	} callbacksInstance;
}

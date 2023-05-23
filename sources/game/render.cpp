#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	const auto gameResetListener = eventGameReset().listen([]() {
		engineEntities()->destroy();
	}, -90);

	const auto gameUpdateListener = eventGameUpdate().listen([]() {
		entitiesVisitor([](Entity *e, MovementComponent &mv, EngineComponent &ec) {
			TransformComponent &t = ec.entity->value<TransformComponent>();
			const Real f = mv.timeEnd > mv.timeStart ? saturate(Real(gameTime - mv.timeStart) / (mv.timeEnd - mv.timeStart)) : 0;
			const Vec3 a = globalGrid->center(mv.tileStart);
			const Vec3 b = globalGrid->center(mv.tileEnd);
			t.position = interpolate(a, b, f);
			t.orientation = interpolate(t.orientation, Quat(b - a, Vec3(0, 1, 0)), 0.15);
		}, gameEntities(), false);
	});

	void engineComponentAdded(Entity *e)
	{
		Entity *f = e->value<EngineComponent>().entity = engineEntities()->createAnonymous();
		if (e->has<PositionComponent>())
			f->value<TransformComponent>().position = globalGrid->center(e->value<PositionComponent>().tile);
	}

	void engineComponentRemoved(Entity *e)
	{
		Entity *f = e->value<EngineComponent>().entity;
		if (f)
			f->destroy();
	}

	struct Callbacks
	{
		EventListener<bool(Entity *)> engineComponentAddedListener;
		EventListener<bool(Entity *)> engineComponentRemovedListener;

		Callbacks()
		{
			EntityComponent *ec = gameEntities()->component<EngineComponent>();
			engineComponentAddedListener.attach(ec->group()->entityAdded);
			engineComponentAddedListener.bind(&engineComponentAdded);
			engineComponentRemovedListener.attach(ec->group()->entityRemoved);
			engineComponentRemovedListener.bind(&engineComponentRemoved);
		}
	} callbacksInstance;
}

#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

extern bool ortho;

namespace
{
	void gameUpdate()
	{
		Entity *cam = engineEntities()->get(1);
		const TransformComponent &ct = cam->value<TransformComponent>();
		entitiesVisitor([&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const HealthbarComponent &hc) {
			TransformComponent &t = hc.entity->value<TransformComponent>();
			t.position = mv.position() + Vec3(0, 0.3, 0);
			t.orientation = Quat(t.position - ct.position, ct.orientation * Vec3(0, 1, 0));
			if (ortho)
				t.position += Vec3(0, 1, 0);
			else
				t.position += t.orientation * Vec3(0, 0, 1);
			hc.entity->value<TextureAnimationComponent>().offset = 1 - Real(mo.life) / mo.maxLife;
		}, gameEntities(), false);
	}

	void healthbarComponentAdded(Entity *e)
	{
		Entity *f = e->value<HealthbarComponent>().entity = engineEntities()->createAnonymous();
		f->value<RenderComponent>().object = HashString("mazetd/misc/healthbar.obj");
		if (e->has<PositionComponent>())
			f->value<TransformComponent>().position = globalGrid->center(e->value<PositionComponent>().tile);
		f->value<TextureAnimationComponent>().speed = 0;
	}

	void healthbarComponentRemoved(Entity *e)
	{
		Entity *f = e->value<HealthbarComponent>().entity;
		if (f)
			f->destroy();
	}

	struct Callbacks
	{
		EventListener<void()> gameUpdateListener;
		EventListener<void(Entity *)> healthbarComponentAddedListener;
		EventListener<void(Entity *)> healthbarComponentRemovedListener;

		Callbacks()
		{
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
			{
				EntityComponent *ec = gameEntities()->component<HealthbarComponent>();
				healthbarComponentAddedListener.attach(ec->group()->entityAdded);
				healthbarComponentAddedListener.bind<&healthbarComponentAdded>();
				healthbarComponentRemovedListener.attach(ec->group()->entityRemoved);
				healthbarComponentRemovedListener.bind<&healthbarComponentRemoved>();
			}
		}
	} callbacksInstance;
}

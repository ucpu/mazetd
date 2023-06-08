#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

extern bool ortho;

namespace
{
	const auto gameUpdateListener = eventGameUpdate().listen(
		[]()
		{
			Entity *cam = engineEntities()->get(1);
			const TransformComponent &ct = cam->value<TransformComponent>();

			entitiesVisitor(
				[&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const HealthbarComponent &hc)
				{
					TransformComponent &t = hc.entity->value<TransformComponent>();
					t.position = mv.position() + Vec3(0, 0.3, 0);
					t.orientation = Quat(t.position - ct.position, ct.orientation * Vec3(0, 1, 0));
					if (ortho)
						t.position += Vec3(0, 2, 0);
					else
						t.position += t.orientation * Vec3(0, 0, 2);
					hc.entity->value<TextureAnimationComponent>().offset = Real(mo.life) / mo.maxLife;
				},
				gameEntities(), false);

			entitiesVisitor(
				[&](Entity *e)
				{
					if (e->has<ManaStorageComponent>() && e->value<ManaStorageComponent>().capacity > 0)
						e->value<ManabarComponent>();
					else
						e->remove<ManabarComponent>();
				},
				gameEntities(), false);

			entitiesVisitor(
				[&](Entity *e, const PositionComponent &pc, const PivotComponent &pv, const ManaStorageComponent &ms, const ManabarComponent &mc)
				{
					TransformComponent &t = mc.entity->value<TransformComponent>();
					t.position = pc.position() + Vec3(0, pv.elevation * 0.7, 0);
					t.orientation = Quat(t.position - ct.position, ct.orientation * Vec3(0, 1, 0));
					if (ortho)
						t.position += Vec3(0, 2, 0);
					else
						t.position += t.orientation * Vec3(0, 0, 2);
					mc.entity->value<TextureAnimationComponent>().offset = Real(ms.mana) / ms.capacity;
				},
				gameEntities(), false);
		});

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

	void manabarComponentAdded(Entity *e)
	{
		Entity *f = e->value<ManabarComponent>().entity = engineEntities()->createAnonymous();
		f->value<RenderComponent>().object = HashString("mazetd/misc/manabar.obj");
		if (e->has<PositionComponent>())
			f->value<TransformComponent>().position = globalGrid->center(e->value<PositionComponent>().tile);
		f->value<TextureAnimationComponent>().speed = 0;
	}

	void manabarComponentRemoved(Entity *e)
	{
		Entity *f = e->value<ManabarComponent>().entity;
		if (f)
			f->destroy();
	}

	struct Callbacks
	{
		EventListener<bool(Entity *)> healthbarComponentAddedListener;
		EventListener<bool(Entity *)> healthbarComponentRemovedListener;
		EventListener<bool(Entity *)> manabarComponentAddedListener;
		EventListener<bool(Entity *)> manabarComponentRemovedListener;

		Callbacks()
		{
			{
				EntityComponent *ec = gameEntities()->component<HealthbarComponent>();
				healthbarComponentAddedListener.attach(ec->group()->entityAdded);
				healthbarComponentAddedListener.bind(&healthbarComponentAdded);
				healthbarComponentRemovedListener.attach(ec->group()->entityRemoved);
				healthbarComponentRemovedListener.bind(&healthbarComponentRemoved);
			}
			{
				EntityComponent *ec = gameEntities()->component<ManabarComponent>();
				manabarComponentAddedListener.attach(ec->group()->entityAdded);
				manabarComponentAddedListener.bind(&manabarComponentAdded);
				manabarComponentRemovedListener.attach(ec->group()->entityRemoved);
				manabarComponentRemovedListener.bind(&manabarComponentRemoved);
			}
		}
	} callbacksInstance;
}

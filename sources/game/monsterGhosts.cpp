#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"

namespace
{
	struct GhostComponent
	{
		quat rot;
		vec3 mov;
	};

	void engineInit()
	{
		engineEntities()->defineComponent(GhostComponent());
	}

	void engineUpdate()
	{
		entitiesVisitor([&](Entity *e, TransformComponent &tr, RenderComponent &re, const GhostComponent &gh) {
			tr.orientation = gh.rot * tr.orientation;
			tr.position += gh.mov;
			re.opacity -= 0.004;
			if (re.opacity < 0.02)
				e->destroy();
		}, engineEntities(), true);
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

void createMonsterGhost(Entity *ge)
{
	Entity *f = ge->value<EngineComponent>().entity;
	Entity *ee = engineEntities()->createAnonymous();
	TransformComponent &tr = ee->value<TransformComponent>();
	tr = f->value<TransformComponent>();
	tr.orientation = interpolate(quat(), randomDirectionQuat(), 0.1) * tr.orientation;
	SkeletalAnimationComponent &sk = ee->value<SkeletalAnimationComponent>();
	sk = f->value<SkeletalAnimationComponent>();
	RenderComponent &re = ee->value<RenderComponent>();
	re = f->value<RenderComponent>();
	re.opacity = 0.5;
	GhostComponent &gh = ee->value<GhostComponent>();
	gh.rot = interpolate(quat(), randomDirectionQuat(), 0.01);
	gh.mov = (vec3(0, 5, 0) + randomDirection3()) * 0.01;
}

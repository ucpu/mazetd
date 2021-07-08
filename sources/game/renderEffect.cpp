#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/engine.h>

#include "../game.h"

namespace
{
	struct RenderEffectComponent
	{
		vec3 move;
		uint64 timeToDie = 0;
	};

	void engineInit()
	{
		engineEntities()->defineComponent(RenderEffectComponent());
	}

	void engineUpdate()
	{
		const uint64 time = engineControlTime();
		entitiesVisitor(engineEntities(), [&](Entity *e, TransformComponent &tr, const RenderEffectComponent &re) {
			tr.position += re.move;
			if (time > re.timeToDie)
				e->destroy();
		}, true);
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

void renderEffect(const EffectConfig &config)
{
	const uint64 step = controlThread().updatePeriod();
	const real dist = distance(config.pos1, config.pos2);
	const vec3 dir = normalize(config.pos2 - config.pos1);
	const uint32 n = dist.value;
	for (uint32 i = 0; i < n; i++)
	{
		Entity *e = engineEntities()->createAnonymous();
		RenderEffectComponent &re = e->value<RenderEffectComponent>();
		const uint64 dur = randomRange(0.05, 0.15) * 1e6;
		re.timeToDie = engineControlTime() + dur;
		re.move = dir * (step / (real)dur);
		CAGE_COMPONENT_ENGINE(Transform, tr, e);
		tr.position = config.pos1 + dir * (i + 0.5);
		tr.scale = 0.05;
		tr.orientation = randomDirectionQuat();
		CAGE_COMPONENT_ENGINE(Render, r, e);
		r.object = HashString("cage/model/fake.obj");
	}
}

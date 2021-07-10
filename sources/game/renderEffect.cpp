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
		Entity *cam = engineEntities()->get(1);
		CAGE_COMPONENT_ENGINE(Transform, ct, cam);
		entitiesVisitor(engineEntities(), [&](Entity *e, TransformComponent &tr, const RenderEffectComponent &re) {
			tr.position += re.move;
			tr.orientation = quat(tr.position - ct.position, ct.orientation * vec3(0, 1, 0));
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

	uint32 renderName(const EffectConfig &config)
	{
		switch (config.type)
		{
		case EffectTypeEnum::Physical: return HashString("mazetd/particles/sprite.obj;physical");
		case EffectTypeEnum::Fire: return HashString("mazetd/particles/sprite.obj;fire");
		case EffectTypeEnum::Water: return HashString("mazetd/particles/sprite.obj;water");
		case EffectTypeEnum::Poison: return HashString("mazetd/particles/sprite.obj;poison");
		case EffectTypeEnum::Mana: return HashString("mazetd/particles/sprite.obj;mana");
		default: return HashString("cage/model/fake.obj");
		}
	}
}

void renderEffect(const EffectConfig &config)
{
	if (config.type == EffectTypeEnum::None)
		return;
	const uint64 step = controlThread().updatePeriod();
	const real dist = distance(config.pos1, config.pos2);
	const vec3 dir = normalize(config.pos2 - config.pos1);
	const uint32 n = max(uint32(dist.value), 1u);
	Entity *cam = engineEntities()->get(1);
	CAGE_COMPONENT_ENGINE(Transform, ct, cam);
	for (uint32 i = 0; i < n; i++)
	{
		Entity *e = engineEntities()->createAnonymous();
		RenderEffectComponent &re = e->value<RenderEffectComponent>();
		const uint64 dur = randomRange(0.05, 0.15) * 1e6;
		re.timeToDie = engineControlTime() + dur;
		re.move = dir * (step / (real)dur);
		CAGE_COMPONENT_ENGINE(Transform, tr, e);
		tr.position = config.pos1 + dir * (i + 0.2);
		tr.orientation = quat(tr.position - ct.position, ct.orientation * vec3(0, 1, 0));
		CAGE_COMPONENT_ENGINE(Render, r, e);
		r.object = renderName(config);
		CAGE_COMPONENT_ENGINE(TextureAnimation, ta, e);
		ta.startTime = randomRange(0u, 1000000000u);
	}
}

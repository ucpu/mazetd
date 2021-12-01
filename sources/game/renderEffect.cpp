#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"

namespace
{
	struct RenderEffectComponent
	{
		Vec3 move;
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
		TransformComponent &ct = cam->value<TransformComponent>();
		entitiesVisitor([&](Entity *e, TransformComponent &tr, const RenderEffectComponent &re) {
			tr.position += re.move;
			tr.orientation = Quat(tr.position - ct.position, ct.orientation * Vec3(0, 1, 0));
			if (time > re.timeToDie)
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

	uint32 renderName(const EffectConfig &config)
	{
		switch (config.type)
		{
		case DamageTypeEnum::Physical: return HashString("mazetd/particles/sprite.obj;physical");
		case DamageTypeEnum::Fire: return HashString("mazetd/particles/sprite.obj;fire");
		case DamageTypeEnum::Water: return HashString("mazetd/particles/sprite.obj;water");
		case DamageTypeEnum::Poison: return HashString("mazetd/particles/sprite.obj;poison");
		case DamageTypeEnum::Magic: return HashString("mazetd/particles/sprite.obj;magic");
		default: return HashString("cage/model/fake.obj");
		}
	}
}

void renderEffect(const EffectConfig &config)
{
	if (config.type >= DamageTypeEnum::Total)
		return;
	const uint64 step = controlThread().updatePeriod();
	const Real dist = distance(config.pos1, config.pos2);
	const Vec3 dir = normalize(config.pos2 - config.pos1);
	const uint32 n = max(uint32(dist.value), 1u);
	Entity *cam = engineEntities()->get(1);
	TransformComponent &ct = cam->value<TransformComponent>();
	for (uint32 i = 0; i < n; i++)
	{
		Entity *e = engineEntities()->createAnonymous();
		RenderEffectComponent &re = e->value<RenderEffectComponent>();
		const uint64 dur = randomRange(0.05, 0.15) * 1e6;
		re.timeToDie = engineControlTime() + dur;
		re.move = dir * (step / (Real)dur);
		TransformComponent &tr = e->value<TransformComponent>();
		tr.position = config.pos1 + dir * (i + 0.2);
		tr.orientation = Quat(tr.position - ct.position, ct.orientation * Vec3(0, 1, 0));
		RenderComponent &r = e->value<RenderComponent>();
		r.object = renderName(config);
		TextureAnimationComponent &ta = e->value<TextureAnimationComponent>();
		ta.startTime = randomRange(0u, 1000000000u);
	}
}

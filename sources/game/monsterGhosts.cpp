#include "../game.h"

#include <cage-core/entitiesVisitor.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

namespace mazetd
{
	namespace
	{
		struct GhostComponent
		{
			Quat rot;
			Vec3 mov;
		};

		const auto engineInitListener = controlThread().initialize.listen([]() { engineEntities()->defineComponent(GhostComponent()); });

		const auto engineUpdateListener = controlThread().update.listen(
			[]()
			{
				entitiesVisitor(
					[&](Entity *e, TransformComponent &tr, const GhostComponent &gh)
					{
						tr.orientation = gh.rot * tr.orientation;
						tr.position += gh.mov;
						tr.scale *= 0.98;
						if (tr.scale < 0.1)
							e->destroy();
					},
					engineEntities(), true);
			});
	}

	void createMonsterGhost(Entity *ge)
	{
		Entity *f = ge->value<EngineComponent>().entity;
		Entity *ee = engineEntities()->createAnonymous();
		TransformComponent &tr = ee->value<TransformComponent>();
		tr = f->value<TransformComponent>();
		tr.orientation = interpolate(Quat(), randomDirectionQuat(), 0.1) * tr.orientation;
		SkeletalAnimationComponent &sk = ee->value<SkeletalAnimationComponent>();
		sk = f->value<SkeletalAnimationComponent>();
		RenderComponent &re = ee->value<RenderComponent>();
		re = f->value<RenderComponent>();
		re.opacity = 0.55;
		GhostComponent &gh = ee->value<GhostComponent>();
		gh.rot = interpolate(Quat(), randomDirectionQuat(), 0.01);
		gh.mov = (Vec3(0, 5, 0) + randomDirection3()) * 0.01;
	}
}

#include <cage-core/hashString.h>
#include <cage-core/enumerate.h>
#include <cage-core/color.h>
#include <cage-engine/scene.h>
#include <cage-engine/window.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	struct PathMarkComponent
	{};

	void engineInit()
	{
		engineEntities()->defineComponent(PathMarkComponent());
	}

	void engineUpdate()
	{
		engineEntities()->component<PathMarkComponent>()->destroy();
		if (!globalWaypoints)
			return;
		const auto &wp = globalWaypoints->waypoints[globalWaypoints->minDistanceSpawner];
		const Real colorIndexScale = 1 / Real(wp->fullPath.size());
		const Real animationFactor = ((engineControlTime() * gameSpeed) % 1e6) / 1e6;
		uint32 prev = m;
		for (const auto it : enumerate(wp->fullPath))
		{
			if (prev != m)
			{
				Entity *e = engineEntities()->createAnonymous();
				e->value<PathMarkComponent>();
				TransformComponent &t = e->value<TransformComponent>();
				const Vec3 a = globalGrid->center(prev);
				const Vec3 b = globalGrid->center(*it);
				t.position = interpolate(a, b, animationFactor);
				t.orientation = Quat(b - a, Vec3(0, 1, 0));
				t.scale = 0.5;
				RenderComponent &r = e->value<RenderComponent>();
				r.object = HashString("mazetd/misc/pathMark.obj");
				r.color = colorValueToHeatmapRgb(Real(it.index) * colorIndexScale);
				std::swap(r.color[1], r.color[2]);
			}
			prev = *it;
		}
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

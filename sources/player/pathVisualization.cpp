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
	uint64 timeToDestroy;

	InputListener<InputClassEnum::KeyRepeat, InputKey, bool> keyRepeatListener;

	struct PathMarkComponent
	{};

	void destroyMarks()
	{
		engineEntities()->component<PathMarkComponent>()->destroy();
	}

	void placeMarks()
	{
		destroyMarks();
		timeToDestroy = engineControlTime() + 5 * 1000000;
		if (!globalWaypoints)
			return;
		const auto &wp = globalWaypoints->waypoints[globalWaypoints->minDistanceSpawner];
		uint32 prev = m;
		const Real colorIndexScale = 1 / Real(wp->fullPath.size());
		for (const auto it : enumerate(wp->fullPath))
		{
			if (prev != m)
			{
				Entity *e = engineEntities()->createAnonymous();
				e->value<PathMarkComponent>();
				TransformComponent &t = e->value<TransformComponent>();
				const Vec3 a = globalGrid->center(prev);
				const Vec3 b = globalGrid->center(*it);
				t.position = interpolate(a, b, 0.5);
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

	void gameUpdate()
	{
		if (gameTime == 1)
			placeMarks();
	}

	bool keyRepeat(InputKey in)
	{
		if (in.key != 80) // key P
			return false;
		placeMarks();
		return true;
	}

	void engineInit()
	{
		engineEntities()->defineComponent(PathMarkComponent());

		keyRepeatListener.attach(engineWindow()->events, 200);
		keyRepeatListener.bind<&keyRepeat>();
	}

	void engineUpdate()
	{
		if (engineControlTime() >= timeToDestroy)
			destroyMarks();
	}

	struct Callbacks
	{
		EventListener<void()> gameUpdateListener;
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

void destroyShortestPathVisualizationMarks()
{
	timeToDestroy = 0;
}

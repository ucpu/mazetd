#include <cage-core/hashString.h>
#include <cage-core/enumerate.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	uint64 timeToDestroy;
	uint32 waypointIndex;

	WindowEventListeners listeners;

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
		const auto &wp = globalWaypoints->waypoints[waypointIndex % globalWaypoints->waypoints.size()];
		uint32 prev = m;
		for (uint32 tile : wp->fullPath)
		{
			if (prev != m)
			{
				Entity *e = engineEntities()->createAnonymous();
				e->value<PathMarkComponent>();
				CAGE_COMPONENT_ENGINE(Transform, t, e);
				const vec3 a = globalGrid->center(prev);
				const vec3 b = globalGrid->center(tile);
				t.position = interpolate(a, b, 0.5);
				t.orientation = quat(b - a, vec3(0, 1, 0));
				CAGE_COMPONENT_ENGINE(Render, r, e);
				r.object = HashString("mazetd/misc/pathMark.obj");
			}
			prev = tile;
		}
		waypointIndex++;
	}

	bool keyRepeat(uint32, uint32 scanCode, ModifiersFlags)
	{
		if (scanCode != 57) // spacebar
			return false;
		placeMarks();
		return true;
	}

	void engineInit()
	{
		engineEntities()->defineComponent(PathMarkComponent());

		listeners.attachAll(engineWindow(), 200);
		listeners.keyRepeat.bind<&keyRepeat>();
	}

	void engineUpdate()
	{
		if (engineControlTime() >= timeToDestroy)
			destroyMarks();
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

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
		if (!globalGrid)
			return;
		uint32 waypointBits = 0;
		uint32 tile = globalPaths->paths[waypointIndex % globalPaths->paths.size()]->tile;
		uint32 prev = m;
		while (tile != m)
		{
			for (const auto &it : enumerate(globalPaths->paths))
				if (it.get()->tile == tile)
					waypointBits |= 1u << it.index;
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
			tile = globalPaths->find(tile, waypointBits).tile;
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

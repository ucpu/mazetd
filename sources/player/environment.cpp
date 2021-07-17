#include <cage-engine/engine.h>

#include "../game.h"

namespace
{
	void gameReset()
	{
		Entity *e = engineEntities()->createAnonymous();
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		t.orientation = quat(degs(-50), randomAngle(), degs());
		CAGE_COMPONENT_ENGINE(Light, l, e);
		l.lightType = LightTypeEnum::Directional;
		l.color = vec3(1);
		l.intensity = 1.5;
		CAGE_COMPONENT_ENGINE(Shadowmap, s, e);
		s.resolution = 4096;
		s.worldSize = vec3(80);
	}

	struct Callbacks
	{
		EventListener<void()> gameResetListener;

		Callbacks()
		{
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
		}
	} callbacksInstance;
}

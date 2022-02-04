#include <cage-engine/scene.h>
#include <cage-simple/engine.h>
#include "../game.h"

namespace
{
	void gameReset()
	{
		Entity *e = engineEntities()->createAnonymous();
		TransformComponent &t = e->value<TransformComponent>();
		t.orientation = Quat(Degs(-50), randomAngle(), Degs());
		LightComponent &l = e->value<LightComponent>();
		l.lightType = LightTypeEnum::Directional;
		l.color = Vec3(1);
		l.intensity = 1.5;
		ShadowmapComponent &s = e->value<ShadowmapComponent>();
		s.resolution = 4096;
		s.worldSize = Vec3(80);
	}

	void gameUpdate()
	{
		if ((gameTime % 15) == 0)
			playerMoney += 1;
	}

	struct Callbacks
	{
		EventListener<void()> gameResetListener;
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

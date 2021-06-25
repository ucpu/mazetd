#include <cage-core/entities.h>
#include <cage-engine/engine.h>
#include <cage-engine/fpsCamera.h>

using namespace cage;

namespace
{
	Holder<FpsCamera> fps;

	void engineInit()
	{
		Entity *e = engineEntities()->create(1);
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		t.position = vec3(0, 50, 50);
		t.orientation = quat(degs(-45), degs(), degs());
		CAGE_COMPONENT_ENGINE(Camera, c, e);
		c.near = 1;
		c.far = 500;
		c.ambientColor = vec3(1);
		c.ambientIntensity = 1;
		c.ambientDirectionalColor = vec3(1);
		c.ambientDirectionalIntensity = 0.1;

		fps = newFpsCamera(e);
		fps->mouseButton = MouseButtonsFlags::Middle;
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
		}
	} callbacksInstance;
}

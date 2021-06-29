#include <cage-core/entities.h>
#include <cage-engine/engine.h>

using namespace cage;

namespace
{
	void engineInit()
	{
		Entity *e = engineEntities()->createAnonymous();
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		t.orientation = quat(degs(-70), randomAngle(), degs());
		CAGE_COMPONENT_ENGINE(Light, l, e);
		l.lightType = LightTypeEnum::Directional;
		l.color = vec3(1);
		l.intensity = 1;
		CAGE_COMPONENT_ENGINE(Shadowmap, s, e);
		s.resolution = 2048;
		s.worldSize = vec3(80);
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

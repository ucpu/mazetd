#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"

namespace mazetd
{
	namespace
	{
		const auto gameResetListener = eventGameReset().listen(
			[]()
			{
				Entity *e = engineEntities()->createUnique();
				TransformComponent &t = e->value<TransformComponent>();
				t.orientation = Quat(Degs(-60), randomAngle(), Degs());
				LightComponent &l = e->value<LightComponent>();
				l.lightType = LightTypeEnum::Directional;
				l.color = Vec3(1);
				l.intensity = 1.5;
				ShadowmapComponent &s = e->value<ShadowmapComponent>();
				s.resolution = 4096;
				s.directionalWorldSize = 75;
			});

		const auto gameUpdateListener = eventGameUpdate().listen(
			[]()
			{
				if ((gameTime % 15) == 0)
					playerMoney += 1;
			});
	}
}

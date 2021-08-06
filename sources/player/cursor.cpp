#include <cage-core/camera.h>
#include <cage-core/geometry.h>
#include <cage-core/collider.h>
#include <cage-core/hashString.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	Entity *cursorMarker = nullptr;

	void engineUpdate()
	{
		const ivec2 res = engineWindow()->resolution();
		if (gameRunning && globalGrid && !playerPanning && engineWindow()->isFocused() && res[0] > 0 && res[1] > 0)
		{
			const ivec2 cur = engineWindow()->mousePosition();
			Entity *c = engineEntities()->component<CameraComponent>()->entities()[0];
			TransformComponent &t = c->value<TransformComponent>();
			CameraComponent &a = c->value<CameraComponent>();
			const mat4 view = mat4(inverse(t));
			const mat4 proj = perspectiveProjection(a.camera.perspectiveFov, real(res[0]) / real(res[1]), a.near, a.far);
			const mat4 inv = inverse(proj * view);
			const vec2 cp = (vec2(cur) / vec2(res) * 2 - 1) * vec2(1, -1);
			const vec4 pn = inv * vec4(cp, -1, 1);
			const vec4 pf = inv * vec4(cp, 1, 1);
			const vec3 near = vec3(pn) / pn[3];
			const vec3 far = vec3(pf) / pf[3];
			const Line line = makeSegment(near, far);
			playerCursorPosition = intersection(line, +globalCollider, transform());
			if (playerCursorPosition.valid())
			{
				playerCursorTile = globalGrid->index(playerCursorPosition);
				if (playerCursorTile != m && none(globalGrid->flags[playerCursorTile] & TileFlags::Invalid))
				{
					if (!cursorMarker)
					{
						cursorMarker = engineEntities()->createAnonymous();
						RenderComponent &r = cursorMarker->value<RenderComponent>();
						r.object = HashString("mazetd/misc/cursor.obj");
					}
					TransformComponent &t = cursorMarker->value<TransformComponent>();
					t.position = globalGrid->center(playerCursorTile);
					return;
				}
			}
		}
		if (cursorMarker)
		{
			cursorMarker->destroy();
			cursorMarker = nullptr;
		}
		playerCursorTile = m;
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, -100);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

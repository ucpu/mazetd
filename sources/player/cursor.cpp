#include <cage-core/camera.h>
#include <cage-core/geometry.h>
#include <cage-core/collider.h>
#include <cage-core/hashString.h>
#include <cage-engine/window.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	Entity *cursorMarker = nullptr;

	void engineUpdate()
	{
		const Vec2i res = engineWindow()->resolution();
		if (gameRunning && globalGrid && !playerPanning && engineWindow()->isFocused() && res[0] > 0 && res[1] > 0)
		{
			const Vec2i cur = engineWindow()->mousePosition();
			Entity *c = engineEntities()->component<CameraComponent>()->entities()[0];
			TransformComponent &t = c->value<TransformComponent>();
			CameraComponent &a = c->value<CameraComponent>();
			const Mat4 view = Mat4(inverse(t));
			const Mat4 proj = perspectiveProjection(a.camera.perspectiveFov, Real(res[0]) / Real(res[1]), a.near, a.far);
			const Mat4 inv = inverse(proj * view);
			const Vec2 cp = (Vec2(cur) / Vec2(res) * 2 - 1) * Vec2(1, -1);
			const Vec4 pn = inv * Vec4(cp, -1, 1);
			const Vec4 pf = inv * Vec4(cp, 1, 1);
			const Vec3 near = Vec3(pn) / pn[3];
			const Vec3 far = Vec3(pf) / pf[3];
			const Line line = makeSegment(near, far);
			playerCursorPosition = intersection(line, +globalCollider, Transform());
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

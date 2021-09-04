#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../grid.h"
#include "../game.h"

namespace
{
	Vec2i lastMousePos;
	Vec2 camCenter;
	Real camDist = 50;
	Rads camYaw = Degs(45);
	Transform camTrans;

	void updateCamera()
	{
		Real elev = 0;
		if (globalGrid)
		{
			const uint32 index = globalGrid->index(camCenter);
			if (index != m)
				elev = globalGrid->center(index)[1];
		}
		const Real cd = saturate((camDist - 10) / (70 - 10));
		const Rads pitch = interpolate(Degs(-75), Degs(-55), smoothstep(cd));
		camTrans.orientation = Quat(pitch, camYaw, Degs());
		camTrans.position = Vec3(camCenter[0], elev, camCenter[1]) + camTrans.orientation * Vec3(0, 0, camDist);
	}

	Vec2i centerMouse()
	{
		const Vec2i cntr = engineWindow()->resolution() / 2;
		const Vec2i pos = engineWindow()->mousePosition();
		engineWindow()->mousePosition(cntr);
		return pos - cntr;
	}

	bool mousePress(MouseButtonsFlags button, ModifiersFlags mods, const Vec2i &pos)
	{
		if (button != MouseButtonsFlags::Right)
			return false;
		engineWindow()->mouseVisible(false);
		lastMousePos = pos;
		centerMouse();
		playerPanning = true;
		return true;
	}

	void stop()
	{
		if (playerPanning && engineWindow()->isFocused())
			engineWindow()->mousePosition(lastMousePos);
		engineWindow()->mouseVisible(true);
		playerPanning = false;
	}

	bool mouseRelease(MouseButtonsFlags button, ModifiersFlags, const Vec2i &)
	{
		if (button == MouseButtonsFlags::Right)
			stop();
		return false;
	}

	bool mouseMove(MouseButtonsFlags, ModifiersFlags, const Vec2i &)
	{
		if (!playerPanning)
			return false;
		if (engineWindow()->isFocused())
		{
			const Vec2i mv2 = centerMouse();
			const Vec3 mv3 = Quat(Degs(), camYaw, Degs()) * Vec3(mv2[0], 0, mv2[1]);
			const Real speed = pow(camDist, 0.85) / engineWindow()->contentScaling() * 0.005;
			camCenter += Vec2(mv3[0], mv3[2]) * speed;
			updateCamera();
			return true;
		}
		stop();
		return false;
	}

	bool mouseWheel(sint32 wheel, ModifiersFlags mods, const Vec2i &)
	{
		switch (mods)
		{
		case ModifiersFlags::None:
		{
			camDist *= powE(Real(wheel) * -0.2);
			camDist = clamp(camDist, 5, 70);
			updateCamera();
			return true;
		}
		case ModifiersFlags::Ctrl:
		{
			camYaw += Degs(wheel) * 5;
			updateCamera();
			return true;
		}
		}
		return false;
	}

	bool focusLose()
	{
		stop();
		return false;
	}

	WindowEventListeners listeners;

	void engineInit()
	{
		listeners.attachAll(engineWindow(), 100);
		listeners.mousePress.bind<&mousePress>();
		listeners.mouseRelease.bind<&mouseRelease>();
		listeners.mouseMove.bind<&mouseMove>();
		listeners.mouseWheel.bind<&mouseWheel>();
		listeners.focusLose.bind<&focusLose>();
	}

	void engineUpdate()
	{
		if (engineWindow()->isFocused())
		{
			Vec2 mv2;
			if (engineWindow()->keyboardKey(87)) // w
				mv2[1] -= 1;
			if (engineWindow()->keyboardKey(83)) // s
				mv2[1] += 1;
			if (engineWindow()->keyboardKey(65)) // a
				mv2[0] -= 1;
			if (engineWindow()->keyboardKey(68)) // d
				mv2[0] += 1;
			if (mv2 != Vec2())
			{
				mv2 = normalize(mv2);
				const Vec3 mv3 = Quat(Degs(), camYaw, Degs()) * Vec3(mv2[0], 0, mv2[1]);
				const Real speed = pow(camDist, 0.85) * 0.05;
				camCenter += Vec2(mv3[0], mv3[2]) * speed;
				updateCamera();
			}
			Real yaw;
			if (engineWindow()->keyboardKey(81)) // q
				yaw += 1;
			if (engineWindow()->keyboardKey(69)) // e
				yaw -= 1;
			if (yaw != 0)
			{
				camYaw += Degs(yaw) * 2;
				updateCamera();
			}
		}

		Entity *e = engineEntities()->get(1);
		TransformComponent &t = e->value<TransformComponent>();
		t = interpolate(t, camTrans, 0.5);
	}

	void gameReset()
	{
		Entity *e = engineEntities()->create(1);
		updateCamera();
		TransformComponent &t = e->value<TransformComponent>();
		t = camTrans;
		CameraComponent &c = e->value<CameraComponent>();
		c.near = 0.3;
		c.far = 300;
		c.ambientColor = Vec3(1);
		c.ambientIntensity = 0.3;
		c.ambientDirectionalColor = Vec3(1);
		c.ambientDirectionalIntensity = 0.4;
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;
		EventListener<void()> gameResetListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
		}
	} callbacksInstance;
}

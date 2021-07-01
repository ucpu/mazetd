#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../grid.h"
#include "../game.h"

namespace
{
	ivec2 lastMousePos;
	bool panning = false;
	vec2 camCenter;
	real camDist = 50;
	rads camYaw = degs(45);
	transform camTrans;

	void updateCamera()
	{
		real elev = 0;
		if (globalGrid)
		{
			const uint32 index = globalGrid->index(camCenter);
			if (index != m)
				elev = globalGrid->center(index)[1];
		}
		const real cd = saturate((camDist - 10) / (70 - 10));
		const rads pitch = interpolate(degs(-75), degs(-55), smoothstep(cd));
		camTrans.orientation = quat(pitch, camYaw, degs());
		camTrans.position = vec3(camCenter[0], elev, camCenter[1]) + camTrans.orientation * vec3(0, 0, camDist);
	}

	ivec2 centerMouse()
	{
		const ivec2 cntr = engineWindow()->resolution() / 2;
		const ivec2 pos = engineWindow()->mousePosition();
		engineWindow()->mousePosition(cntr);
		return pos - cntr;
	}

	bool mousePress(MouseButtonsFlags button, ModifiersFlags mods, const ivec2 &pos)
	{
		if (button != MouseButtonsFlags::Right)
			return false;
		engineWindow()->mouseVisible(false);
		lastMousePos = pos;
		centerMouse();
		panning = true;
		return true;
	}

	void stop()
	{
		if (panning && engineWindow()->isFocused())
			engineWindow()->mousePosition(lastMousePos);
		engineWindow()->mouseVisible(true);
		panning = false;
	}

	bool mouseRelease(MouseButtonsFlags button, ModifiersFlags, const ivec2 &)
	{
		if (button == MouseButtonsFlags::Right)
			stop();
		return false;
	}

	bool mouseMove(MouseButtonsFlags, ModifiersFlags, const ivec2 &)
	{
		if (!panning)
			return false;
		if (engineWindow()->isFocused())
		{
			const ivec2 mv2 = centerMouse();
			const vec3 mv3 = quat(degs(), camYaw, degs()) * vec3(mv2[0], 0, mv2[1]);
			const real speed = pow(camDist, 0.85) / engineWindow()->contentScaling() * 0.005;
			camCenter += vec2(mv3[0], mv3[2]) * speed;
			updateCamera();
		}
		else
			stop();
		return false;
	}

	bool mouseWheel(sint32 wheel, ModifiersFlags mods, const ivec2 &)
	{
		switch (mods)
		{
		case ModifiersFlags::None:
		{
			camDist *= powE(real(wheel) * -0.2);
			camDist = clamp(camDist, 5, 70);
			updateCamera();
			return true;
		}
		case ModifiersFlags::Ctrl:
		{
			camYaw += degs(wheel) * 5;
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

		Entity *e = engineEntities()->create(1);
		updateCamera();
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		t = camTrans;
		CAGE_COMPONENT_ENGINE(Camera, c, e);
		c.near = 0.3;
		c.far = 300;
		c.ambientColor = vec3(1);
		c.ambientIntensity = 0.3;
		c.ambientDirectionalColor = vec3(1);
		c.ambientDirectionalIntensity = 0.4;
	}

	void engineUpdate()
	{
		if (engineWindow()->isFocused())
		{
			vec2 mv2;
			if (engineWindow()->keyboardScanCode(17)) // w
				mv2[1] -= 1;
			if (engineWindow()->keyboardScanCode(31)) // s
				mv2[1] += 1;
			if (engineWindow()->keyboardScanCode(30)) // a
				mv2[0] -= 1;
			if (engineWindow()->keyboardScanCode(32)) // d
				mv2[0] += 1;
			if (mv2 != vec2())
			{
				mv2 = normalize(mv2);
				const vec3 mv3 = quat(degs(), camYaw, degs()) * vec3(mv2[0], 0, mv2[1]);
				const real speed = pow(camDist, 0.85) * 0.05;
				camCenter += vec2(mv3[0], mv3[2]) * speed;
				updateCamera();
			}
			real yaw;
			if (engineWindow()->keyboardScanCode(16)) // q
				yaw += 1;
			if (engineWindow()->keyboardScanCode(18)) // e
				yaw -= 1;
			if (yaw != 0)
			{
				camYaw += degs(yaw) * 2;
				updateCamera();
			}
		}

		Entity *e = engineEntities()->get(1);
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		t = interpolate(t, camTrans, 0.5);
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

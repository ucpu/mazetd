#include <cage-core/config.h>
#include <cage-engine/scene.h>
#include <cage-engine/window.h>
#include <cage-simple/engine.h>

#include "../grid.h"
#include "../game.h"

bool ortho;

namespace
{
	ConfigBool confInvertCameraMove("mazetd/camera/invert", false);
	Vec2i lastMousePos;
	Vec2 camCenter;
	Real camDist;
	Rads camYaw;
	Transform camTrans;
	bool needReset;

	void updateCamera()
	{
		Real elev = -8.5;
		if (globalGrid)
		{
			const uint32 index = globalGrid->index(camCenter);
			if (index != m)
				elev = globalGrid->center(index)[1];
		}
		const Real cd = saturate((camDist - 10) / (70 - 10));
		const Rads pitch = interpolate(Degs(-70), Degs(-60), smoothstep(cd));
		camTrans.orientation = Quat(ortho ? Rads(Degs(-90)) : pitch, camYaw, Degs());
		camTrans.position = Vec3(camCenter[0], elev, camCenter[1]) + camTrans.orientation * Vec3(0, 0, camDist);
	}

	Vec2i centerMouse()
	{
		const Vec2i cntr = engineWindow()->resolution() / 2;
		const Vec2i pos = engineWindow()->mousePosition();
		engineWindow()->mousePosition(cntr);
		return pos - cntr;
	}

	bool mousePress(InputMouse in)
	{
		if (in.buttons != MouseButtonsFlags::Right)
			return false;
		engineWindow()->mouseVisible(false);
		lastMousePos = in.position;
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

	bool mouseRelease(InputMouse in)
	{
		if (in.buttons == MouseButtonsFlags::Right)
			stop();
		return false;
	}

	bool mouseMove(InputMouse)
	{
		if (!playerPanning)
			return false;
		if (engineWindow()->isFocused())
		{
			const Vec2i mv2 = centerMouse();
			const Vec3 mv3 = Quat(Degs(), camYaw, Degs()) * Vec3(mv2[0], 0, mv2[1]);
			const Real speed = pow(camDist, 0.85) / engineWindow()->contentScaling() * 0.005;
			camCenter += Vec2(mv3[0], mv3[2]) * speed * (confInvertCameraMove ? -1 : 1);
			updateCamera();
			return true;
		}
		stop();
		return false;
	}

	bool mouseWheel(InputMouseWheel in)
	{
		switch (in.mods)
		{
		case ModifiersFlags::None:
		{
			camDist *= powE(Real(in.wheel) * -0.2);
			camDist = clamp(camDist, 5, 70);
			updateCamera();
			return true;
		}
		case ModifiersFlags::Ctrl:
		{
			camYaw += Degs(in.wheel) * 5;
			updateCamera();
			return true;
		}
		}
		return false;
	}

	bool focusLose(InputWindow)
	{
		stop();
		return false;
	}

	bool keyRelease(InputKey in)
	{
		if (in.key == 'C')
		{
			ortho = !ortho;
			updateCamera();
			return true;
		}
		return false;
	}

	InputListener<InputClassEnum::MousePress, InputMouse, bool> mousePressListener;
	InputListener<InputClassEnum::MouseRelease, InputMouse, bool> mouseReleaseListener;
	InputListener<InputClassEnum::MouseMove, InputMouse, bool> mouseMoveListener;
	InputListener<InputClassEnum::MouseWheel, InputMouseWheel, bool> mouseWheelListener;
	InputListener<InputClassEnum::FocusLose, InputWindow, bool> focusloseListener;
	InputListener<InputClassEnum::KeyRelease, InputKey, bool> keyReleaseListener;

	void engineInit()
	{
		mousePressListener.attach(engineWindow()->events, 100);
		mousePressListener.bind<&mousePress>();
		mouseReleaseListener.attach(engineWindow()->events, 101);
		mouseReleaseListener.bind<&mouseRelease>();
		mouseMoveListener.attach(engineWindow()->events, 102);
		mouseMoveListener.bind<&mouseMove>();
		mouseWheelListener.attach(engineWindow()->events, 103);
		mouseWheelListener.bind<&mouseWheel>();
		focusloseListener.attach(engineWindow()->events, 104);
		focusloseListener.bind<&focusLose>();
		keyReleaseListener.attach(engineWindow()->events, 105);
		keyReleaseListener.bind<&keyRelease>();
	}

	void engineUpdate()
	{
		Entity *e = engineEntities()->get(1);

		if (needReset)
		{
			camCenter = {};
			camDist = 50;
			camYaw = Degs(45);
			updateCamera();
			e->value<TransformComponent>() = camTrans;
			if (globalGrid)
				needReset = false;
		}

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

		TransformComponent &t = e->value<TransformComponent>();
		t = interpolate(t, camTrans, 0.5);
		CameraComponent &c = e->value<CameraComponent>();
		c.cameraType = ortho ? CameraTypeEnum::Orthographic : CameraTypeEnum::Perspective;
		if (ortho)
		{
			const Vec2i res = engineWindow()->resolution();
			const Real ratio = Real(res[0]) / Real(res[1]);
			c.camera.orthographicSize = Vec2(camDist * ratio, camDist) * 0.5;
		}
		else
			c.camera.perspectiveFov = Degs(60);
	}

	void gameReset()
	{
		Entity *e = engineEntities()->create(1);
		CameraComponent &c = e->value<CameraComponent>();
		c.near = 0.3;
		c.far = 300;
		c.ambientColor = Vec3(1);
		c.ambientIntensity = 0.1;
		c.ambientDirectionalColor = Vec3(1);
		c.ambientDirectionalIntensity = 0.5;
		needReset = true;
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

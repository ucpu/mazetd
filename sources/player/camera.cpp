#include "../game.h"
#include "../grid.h"

#include <cage-core/config.h>
#include <cage-engine/scene.h>
#include <cage-engine/sceneScreenSpaceEffects.h>
#include <cage-engine/window.h>
#include <cage-simple/engine.h>

namespace mazetd
{
	bool ortho;

	namespace
	{
		const ConfigBool confInvertCameraMove("mazetd/camera/invert", false);
		Vec2 lastMousePos;
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

		Vec2 centerMouse()
		{
			const Vec2 cntr = Vec2(engineWindow()->resolution()) / 2;
			const Vec2 pos = engineWindow()->mousePosition();
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
				const Vec2 mv2 = centerMouse();
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
					camDist *= pow(Real(in.wheel) * -0.2);
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

		EventListener<bool(const GenericInput &)> mousePressListener;
		EventListener<bool(const GenericInput &)> mouseReleaseListener;
		EventListener<bool(const GenericInput &)> mouseMoveListener;
		EventListener<bool(const GenericInput &)> mouseWheelListener;
		EventListener<bool(const GenericInput &)> focusloseListener;
		EventListener<bool(const GenericInput &)> keyReleaseListener;

		const auto engineInitListener = controlThread().initialize.listen(
			[]()
			{
				mousePressListener.attach(engineWindow()->events, 100);
				mousePressListener.bind(inputListener<InputClassEnum::MousePress, InputMouse>(&mousePress));
				mouseReleaseListener.attach(engineWindow()->events, 101);
				mouseReleaseListener.bind(inputListener<InputClassEnum::MouseRelease, InputMouse>(&mouseRelease));
				mouseMoveListener.attach(engineWindow()->events, 102);
				mouseMoveListener.bind(inputListener<InputClassEnum::MouseMove, InputMouse>(&mouseMove));
				mouseWheelListener.attach(engineWindow()->events, 103);
				mouseWheelListener.bind(inputListener<InputClassEnum::MouseWheel, InputMouseWheel>(&mouseWheel));
				focusloseListener.attach(engineWindow()->events, 104);
				focusloseListener.bind(inputListener<InputClassEnum::FocusLose, InputWindow>(&focusLose));
				keyReleaseListener.attach(engineWindow()->events, 105);
				keyReleaseListener.bind(inputListener<InputClassEnum::KeyRelease, InputKey>(&keyRelease));
			});

		const auto engineUpdateListener = controlThread().update.listen(
			[]()
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
			});

		const auto gameResetListener = eventGameReset().listen(
			[]()
			{
				Entity *e = engineEntities()->create(1);
				CameraComponent &c = e->value<CameraComponent>();
				c.near = 0.3;
				c.far = 300;
				c.ambientColor = Vec3(1);
				c.ambientIntensity = 0.2;
				needReset = true;
				//ScreenSpaceEffectsComponent &ef = e->value<ScreenSpaceEffectsComponent>();
				//ef.effects &= ~ScreenSpaceEffectsFlags::EyeAdaptation;
				//ef.eyeAdaptation.nightDesaturate = 0;
			});
	}
}

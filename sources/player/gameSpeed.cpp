#include <cage-core/scheduler.h>
#include <cage-engine/scene.h>
#include <cage-engine/window.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	bool keyPress(InputKey in)
	{
		if (!gameReady)
			return false;
		constexpr Real speedFactor = 1.5;
		switch (in.key)
		{
		case 334: // numeric plus
		case 266: // page up
			gameSpeed = min(gameSpeed * speedFactor, 10.0);
			return true;
		case 333: // numeric minus
		case 267: // page down
			gameSpeed = max(gameSpeed / speedFactor, 0.5);
			return true;
		case 268: // home
			gameSpeed = 1;
			return true;
		case 32: // spacebar
			gamePaused = !gamePaused;
			return true;
		}
		return false;
	}

	void gameScheduleAction()
	{
		if (!gameReady || gamePaused)
			return;
		CAGE_ASSERT(globalGrid);
		eventGameUpdate().dispatch();
	}

	InputListener<InputClassEnum::KeyPress, InputKey, bool> keyPressListener;
	Holder<Schedule> gameUpdateSchedule;

	void engineInit()
	{
		keyPressListener.attach(engineWindow()->events, 110);
		keyPressListener.bind<&keyPress>();

		ScheduleCreateConfig cfg;
		cfg.name = "game update";
		cfg.type = ScheduleTypeEnum::SteadyPeriodic;
		cfg.action.bind<&gameScheduleAction>();
		gameUpdateSchedule = controlThread().scheduler()->newSchedule(cfg);
	}

	void engineFinish()
	{
		gameUpdateSchedule.clear();
	}

	void engineUpdate()
	{
		CAGE_ASSERT(gameSpeed > 0);
		gameUpdateSchedule->period(numeric_cast<uint64>(1000000 / double(gameSpeed.value) / 30));
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineFinishListener;
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineFinishListener.attach(controlThread().finalize);
			engineFinishListener.bind<&engineFinish>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

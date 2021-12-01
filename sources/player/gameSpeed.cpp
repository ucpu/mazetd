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
		switch (in.key)
		{
		case 334: // numeric plus
		case 266: // page up
			gameSpeed = min(gameSpeed + 1, 10u);
			return true;
		case 333: // numeric minus
		case 267: // page down
			gameSpeed = min(gameSpeed, gameSpeed - 1);
			return true;
		}
		return false;
	}

	void gameScheduleAction()
	{
		if (!gameRunning || gameSpeed == 0)
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
		if (gameSpeed > 0)
			gameUpdateSchedule->period(1000000 / gameSpeed / 30);
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

#include <cage-core/scheduler.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	bool keyPress(uint32 key, ModifiersFlags)
	{
		switch (key)
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

	WindowEventListeners listeners;
	Holder<Schedule> gameUpdateSchedule;

	void engineInit()
	{
		listeners.attachAll(engineWindow(), 110);
		listeners.keyPress.bind<&keyPress>();

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

#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"

namespace
{
	bool focusLose()
	{
		gameSpeed = min(gameSpeed, 1u);
		return true;
	}

	bool keyPress(uint32, uint32 scanCode, ModifiersFlags)
	{
		switch (scanCode)
		{
		case 78: // numeric plus
		case 329: // page up
			gameSpeed = min(gameSpeed + 1, 5u);
			return true;
		case 74: // numeric minus
		case 337: // page down
			gameSpeed = min(gameSpeed, gameSpeed - 1);
			return true;
		}
		return false;
	}

	WindowEventListeners listeners;

	void engineInit()
	{
		listeners.attachAll(engineWindow(), 110);
		listeners.focusLose.bind<&focusLose>();
		listeners.keyPress.bind<&keyPress>();
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
		}
	} callbacksInstance;
}

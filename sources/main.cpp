#include <cage-core/hashString.h>
#include <cage-core/assetManager.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>
#include <cage-engine/engineProfiling.h>
#include <cage-engine/fullscreenSwitcher.h>
#include <cage-engine/highPerformanceGpuHint.h>

using namespace cage;

namespace
{
	bool windowClose()
	{
		engineStop();
		return true;
	}

	WindowEventListeners listeners;
}

int main(int argc, const char *args[])
{
	try
	{
		engineInitialize(EngineCreateConfig());
		controlThread().updatePeriod(1000000 / 30);

		listeners.attachAll(engineWindow(), 1000);
		listeners.windowClose.bind<&windowClose>();

		engineWindow()->title("MazeTD");
		engineAssets()->add(HashString("mazetd/mazetd.pack"));

		{
			Holder<FullscreenSwitcher> fullscreen = newFullscreenSwitcher({});
			Holder<EngineProfiling> engineProfiling = newEngineProfiling();
			engineProfiling->profilingScope = EngineProfilingScopeEnum::None;

			engineStart();
		}

		engineAssets()->remove(HashString("mazetd/mazetd.pack"));
		engineFinalize();
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}

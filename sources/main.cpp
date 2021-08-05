#include <cage-core/hashString.h>
#include <cage-core/assetManager.h>
#include <cage-core/logger.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>
#include <cage-engine/engineStatistics.h>
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
	Holder<Logger> log = newLogger();
	log->format.bind<logFormatConsole>();
	log->output.bind<logOutputStdOut>();

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
			Holder<EngineStatistics> engineStatistics = newEngineStatistics();
			engineStatistics->statisticsScope = EngineStatisticsScopeEnum::None;

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

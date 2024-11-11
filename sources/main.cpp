#include <cage-core/assetsManager.h>
#include <cage-core/hashString.h>
#include <cage-core/logger.h>
#include <cage-engine/highPerformanceGpuHint.h>
#include <cage-engine/window.h>
#include <cage-simple/engine.h>
#include <cage-simple/fullscreenSwitcher.h>
#include <cage-simple/statisticsGui.h>

using namespace cage;

int main(int argc, const char *args[])
{
	try
	{
		initializeConsoleLogger();
		engineInitialize(EngineCreateConfig());
		controlThread().updatePeriod(1000000 / 30);

		const auto closeListener = engineWindow()->events.listen(inputFilter([](input::WindowClose) { engineStop(); }));
		engineWindow()->title("MazeTD");
		engineAssets()->load(HashString("mazetd/mazetd.pack"));

		{
			Holder<FullscreenSwitcher> fullscreen = newFullscreenSwitcher({});
			Holder<StatisticsGui> engineStatistics = newStatisticsGui();
			engineStatistics->statisticsScope = StatisticsGuiScopeEnum::None;
			engineRun();
		}

		engineAssets()->unload(HashString("mazetd/mazetd.pack"));
		engineFinalize();
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}

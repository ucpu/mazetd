#include <cage-core/hashString.h>
#include <cage-core/assetManager.h>
#include <cage-core/logger.h>
#include <cage-engine/window.h>
#include <cage-engine/highPerformanceGpuHint.h>
#include <cage-simple/engine.h>
#include <cage-simple/statisticsGui.h>
#include <cage-simple/fullscreenSwitcher.h>

using namespace cage;

namespace
{
	void windowClose(InputWindow)
	{
		engineStop();
	}
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

		InputListener<InputClassEnum::WindowClose, InputWindow> closeListener;
		closeListener.attach(engineWindow()->events);
		closeListener.bind<&windowClose>();

		engineWindow()->title("MazeTD");
		engineAssets()->add(HashString("mazetd/mazetd.pack"));

		{
			Holder<FullscreenSwitcher> fullscreen = newFullscreenSwitcher({});
			Holder<StatisticsGui> engineStatistics = newStatisticsGui();
			engineStatistics->statisticsScope = StatisticsGuiScopeEnum::None;

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

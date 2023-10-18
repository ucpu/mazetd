#include "../game.h"
#include "screens.h"

namespace mazetd
{
	void setScreenGame();

	namespace
	{
		void buttonStop()
		{
			eventGameReset().dispatch();
			setScreenMainmenu();
		}
	}

	void setScreenPaused()
	{
		cleanGui();
		Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());
		auto _1 = g->alignment(Vec2(0.8, 0.666));
		auto _2 = g->column();
		{
			auto _ = g->panel().text("Pause");
		}
		g->button().text("Continue").bind<&setScreenGame>();
		g->button().text("Abort").bind<&buttonStop>();
	}
}

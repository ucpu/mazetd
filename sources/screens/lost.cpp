#include "../game.h"
#include "screens.h"

namespace
{
	void buttonStop()
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
	}
}

void setScreenLost()
{
	cleanGui();
	Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());
	auto _1 = g->alignment(Vec2(0.8, 0.666));
	auto _2 = g->column();
	{
		auto _ = g->panel().text("Game Over");
		g->label().text(Stringizer() + "Waves: " + SpawningGroup::waveIndex);
	}
	g->button().text("Close").bind<&buttonStop>();
}

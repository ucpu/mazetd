#include "screens.h"

void setScreenAbout()
{
	cleanGui();
	Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());

	auto a = g->alignment(Vec2(0.5));
	auto c = g->column();
	{
		auto a = g->panel().text("About");
		auto b = g->column();

		static constexpr const char *lines[] = {
			"aMAZEing Tower Defense",
			"Free game",
			"Made by Tomáš Malý",
			"Graphics by Quaternius, Kenney and others",
			"Open source: https://github.com/ucpu/mazetd",
		};

		for (const String &it : lines)
			g->label().text(it);
	}
	{
		auto _ = g->rightRow();
		g->button().text("Close").bind<&setScreenMainmenu>();
	}
}

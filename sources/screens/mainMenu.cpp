#include "screens.h"

namespace mazetd
{
	void setScreenGenerating();
	void setScreenAbout();

	void setScreenMainmenu()
	{
		cleanGui();
		Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());

		{
			auto _ = g->alignment(Vec2(0.45, 0.05));
			g->label().text("aMAZEing Tower Defense").textColor(Vec3(203, 238, 239) / 255).textSize(50);
		}

		{
			auto _1 = g->alignment(Vec2(0.8, 0.666));
			auto _2 = g->panel();
			auto _3 = g->column();
			g->button().text("Start").bind<&setScreenGenerating>();
			g->button().text("About").bind<&setScreenAbout>();
			g->button().text("Quit").bind<&engineStop>();
		}
	}

	namespace
	{
		const auto engineInitListener = controlThread().initialize.listen(&setScreenMainmenu, 999);
	}
}

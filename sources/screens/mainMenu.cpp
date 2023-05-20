#include "screens.h"

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
		auto a = g->alignment(Vec2(0.8, 0.666));
		auto b = g->panel();
		auto c = g->column();
		g->button().text("Start").bind<&setScreenGenerating>();
		g->button().text("About").bind<&setScreenAbout>();
		g->button().text("Quit").bind<&engineStop>();
	}
}

namespace
{
	class Callbacks
	{
		EventListener<void()> engineInitListener;

	public:
		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize, 999);
			engineInitListener.bind<&setScreenMainmenu>();
		}
	} callbacksInstance;
}

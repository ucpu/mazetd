#include "screens.h"
#include "../grid.h"

void setScreenGame();

namespace
{
	EventListener<bool()> engineUpdateListener;
	EventListener<bool()> guiCleanListener;

	void engineUpdate()
	{
		if (!globalGrid)
			return;
		setScreenGame();
	}

	void guiClean()
	{
		engineUpdateListener.detach();
	}
}

void setScreenGenerating()
{
	cleanGui();
	guiCleanListener.attach(eventGuiClean());
	guiCleanListener.bind(&guiClean);
	engineUpdateListener.attach(controlThread().update);
	engineUpdateListener.bind(&engineUpdate);

	Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());
	{
		auto a = g->alignment(Vec2(0.5));
		auto b = g->panel();
		g->label().text("Generating...");
	}
}

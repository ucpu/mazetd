#include "../grid.h"
#include "screens.h"

namespace mazetd
{
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
			auto _1 = g->alignment(Vec2(0.5));
			auto _2 = g->panel();
			g->label().text("Generating...");
		}
	}
}

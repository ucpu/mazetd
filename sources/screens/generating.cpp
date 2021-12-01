#include "screens.h"
#include "../grid.h"

void setScreenGame();

namespace
{
	EventListener<void()> engineUpdateListener;
	EventListener<void()> guiCleanListener;

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
	EntityManager *ents = engineGuiEntities();
	guiCleanListener.attach(eventGuiClean());
	guiCleanListener.bind<&guiClean>();
	engineUpdateListener.attach(controlThread().update);
	engineUpdateListener.bind<&engineUpdate>();

	{
		Entity *e = ents->create(1);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 1;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Generating";
	}
}

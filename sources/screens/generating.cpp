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
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		e->value<GuiParentComponent>().parent = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "Generating";
	}
}

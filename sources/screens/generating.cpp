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
	EntityManager *ents = engineGui()->entities();
	guiCleanListener.attach(eventGuiClean());
	guiCleanListener.bind<&guiClean>();
	engineUpdateListener.attach(controlThread().update);
	engineUpdateListener.bind<&engineUpdate>();

	{
		Entity *e = ents->create(1);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 1;
		CAGE_COMPONENT_GUI(Label, label, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Generating";
	}
}

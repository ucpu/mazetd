#include "screens.h"
#include "../game.h"

void setScreenGame();

namespace
{
	bool buttonResume(uint32)
	{
		setScreenGame();
		return true;
	}

	bool buttonStop(uint32)
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
		return true;
	}
}

void setScreenGameMenu()
{
	cleanGui();
	EntityManager *ents = engineGui()->entities();

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
		txt.value = "Pause";
	}

	{
		Entity *e = ents->create(3);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.8, 0.666);
	}

	{
		Entity *e = ents->create(4);
		CAGE_COMPONENT_GUI(Panel, panel, e);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 3;
		CAGE_COMPONENT_GUI(LayoutLine, layout, e);
		layout.vertical = true;
	}

	{
		Entity *e = ents->create(5);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		parent.order = 1;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Resume";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonResume>();
	}

	{
		Entity *e = ents->create(6);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		parent.order = 2;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Stop";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonStop>();
	}
}

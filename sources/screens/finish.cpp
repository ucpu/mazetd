#include "screens.h"

namespace
{
	bool buttonContinue(uint32)
	{
		setScreenMainmenu();
		return true;
	}
}

void setScreenFinish()
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
		txt.value = "Done";
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
		txt.value = "Continue";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonContinue>();
	}
}

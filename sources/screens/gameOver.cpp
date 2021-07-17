#include "screens.h"
#include "../game.h"

namespace
{
	bool buttonContinue(uint32)
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
		return true;
	}
}

void setScreenGameOver()
{
	cleanGui();
	EntityManager *ents = engineGui()->entities();

	{
		Entity *e = ents->create(1);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0, 1);
	}

	{
		Entity *e = ents->create(2);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 1;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Continue";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonContinue>();
	}

	{
		Entity *e = ents->create(3);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.45, 0.05);
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
		ll.vertical = true;
	}

	{
		Entity *e = ents->create(4);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 3;
		parent.order = 0;
		CAGE_COMPONENT_GUI(Label, label, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Game Over";
	}

	{
		Entity *e = ents->create(5);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 3;
		parent.order = 1;
		CAGE_COMPONENT_GUI(Label, label, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = stringizer() + "Waves: " + SpawningGroup::groupIndex;
	}
}

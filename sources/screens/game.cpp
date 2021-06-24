#include <cage-core/entities.h>
#include <cage-engine/engine.h>
#include <cage-engine/gui.h>

#include "../map.h"

void screenGame()
{
	EntityManager *ents = engineGui()->entities();

	{
		Entity *e = ents->create(1);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.5, 0.5);
	}

	{
		Entity *e = ents->create(2);
		CAGE_COMPONENT_GUI(Panel, panel, e);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 1;
		CAGE_COMPONENT_GUI(LayoutLine, layout, e);
		layout.vertical = true;
	}

	{
		Entity *e = ents->create(3);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 2;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Game";
	}
}

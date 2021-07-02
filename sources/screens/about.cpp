#include "screens.h"

namespace
{
	bool buttonBack(uint32)
	{
		setScreenMainmenu();
		return true;
	}
}

void setScreenAbout()
{
	cleanGui();
	EntityManager *ents = engineGui()->entities();

	{
		Entity *e = ents->create(1);
		CAGE_COMPONENT_GUI(LayoutSplitter, l, e);
		l.vertical = true;
		l.inverse = true;
	}

	{
		Entity *e = ents->create(2);
		CAGE_COMPONENT_GUI(Parent, p, e);
		p.parent = 1;
		p.order = 2;
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0, 0.5);
	}

	{
		Entity *e = ents->create(3);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 2;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Back";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonBack>();
	}

	{
		Entity *e = ents->create(4);
		CAGE_COMPONENT_GUI(Parent, p, e);
		p.parent = 1;
		p.order = 1;
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.45, 0.45);
	}

	{
		Entity *e = ents->create(5);
		CAGE_COMPONENT_GUI(Panel, panel, e);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		CAGE_COMPONENT_GUI(LayoutLine, layout, e);
		layout.vertical = true;
	}

	constexpr const char *lines[] = {
		"About",
		"aMAZEing",
		"tower",
		"defense",
		"game",
	};

	uint32 idx = 0;
	for (const auto &it : lines)
	{
		Entity *e = engineGui()->entities()->createUnique();
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 5;
		parent.order = idx;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = it;
		idx++;
	}
}

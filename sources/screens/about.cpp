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
	EntityManager *ents = engineGuiEntities();

	{
		Entity *e = ents->create(1);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0, 1);
	}

	{
		Entity *e = ents->create(2);
		e->value<GuiParentComponent>().parent = 1;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "Back";
		e->value<GuiEventComponent>().event.bind<&buttonBack>();
	}

	{
		Entity *e = ents->create(4);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0.5, 0.2);
	}

	{
		Entity *e = ents->create(5);
		e->value<GuiPanelComponent>();
		e->value<GuiParentComponent>().parent = 4;
		e->value<GuiLayoutLineComponent>().vertical = true;
	}

	static constexpr const char *lines[] = {
		"aMAZEing Tower Defense",
		"Free game",
		"Made by Tomáš Malý",
		"Graphics by Quaternius, Kenney and others",
		"Open source: https://github.com/ucpu/mazetd",
	};

	uint32 idx = 0;
	for (const auto &it : lines)
	{
		Entity *e = engineGuiEntities()->createUnique();
		e->value<GuiParentComponent>().parent = 5;
		e->value<GuiParentComponent>().order = idx++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = it;
	}
}

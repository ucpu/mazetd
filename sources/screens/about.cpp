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
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0, 1);
	}

	{
		Entity *e = ents->create(2);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 1;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Back";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonBack>();
	}

	{
		Entity *e = ents->create(4);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.5, 0.2);
	}

	{
		Entity *e = ents->create(5);
		GuiPanelComponent &panel = e->value<GuiPanelComponent>();
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 4;
		GuiLayoutLineComponent &layout = e->value<GuiLayoutLineComponent>();
		layout.vertical = true;
	}

	constexpr const char *lines[] = {
		"aMAZEing Tower Defense",
		"Free game",
		"Made by Tomáš Malý",
		"Graphics by Quaternius, Kenney and others",
		"Open source: https://github.com/ucpu/mazetd",
		"",
		"LMB - placing selected building",
		"MMB - destroy building",
		"RMB - move camera",
		"Wheel - zoom",
		"WSAD - move camera",
		"QE - rotate camera",
		"Spacebar - visualize monsters paths",
		"PGUP/DN - speed up/down the game",
	};

	uint32 idx = 0;
	for (const auto &it : lines)
	{
		Entity *e = engineGui()->entities()->createUnique();
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 5;
		parent.order = idx;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = it;
		idx++;
	}
}

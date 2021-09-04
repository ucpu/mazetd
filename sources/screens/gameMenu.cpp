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
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 1;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Pause";
	}

	{
		Entity *e = ents->create(3);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.8, 0.666);
	}

	{
		Entity *e = ents->create(4);
		GuiPanelComponent &panel = e->value<GuiPanelComponent>();
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 3;
		GuiLayoutLineComponent &layout = e->value<GuiLayoutLineComponent>();
		layout.vertical = true;
	}

	{
		Entity *e = ents->create(5);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 4;
		parent.order = 1;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Resume";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonResume>();
	}

	{
		Entity *e = ents->create(6);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 4;
		parent.order = 2;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Stop";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonStop>();
	}
}

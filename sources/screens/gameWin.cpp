#include "screens.h"
#include "../game.h"

void setScreenGame();

namespace
{
	bool buttonStop(uint32)
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
		return true;
	}

	bool buttonContinue(uint32)
	{
		setScreenGame();
		return true;
	}
}

void setScreenGameWin()
{
	cleanGui();
	EntityManager *ents = engineGuiEntities();

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
		txt.value = "Stop";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonStop>();
	}

	{
		Entity *e = ents->create(11);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(1, 1);
	}

	{
		Entity *e = ents->create(12);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 11;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Continue";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonContinue>();
	}

	{
		Entity *e = ents->create(3);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.45, 0.05);
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
	}

	{
		Entity *e = ents->create(4);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 3;
		parent.order = 0;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Victory";
	}

	{
		Entity *e = ents->create(5);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 3;
		parent.order = 1;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = Stringizer() + "Waves: " + SpawningGroup::waveIndex;
	}
}

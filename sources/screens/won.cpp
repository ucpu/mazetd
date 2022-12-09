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

void setScreenWon()
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
		e->value<GuiTextComponent>().value = "Stop";
		e->value<GuiEventComponent>().event.bind<&buttonStop>();
	}

	{
		Entity *e = ents->create(11);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(1, 1);
	}

	{
		Entity *e = ents->create(12);
		e->value<GuiParentComponent>().parent = 11;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "Continue";
		e->value<GuiEventComponent>().event.bind<&buttonContinue>();
	}

	{
		Entity *e = ents->create(3);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0.45, 0.05);
		e->value<GuiLayoutLineComponent>().vertical = true;
	}

	{
		Entity *e = ents->create(4);
		e->value<GuiParentComponent>().parent = 3;
		e->value<GuiParentComponent>().order = 0;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "Victory";
	}

	{
		Entity *e = ents->create(5);
		e->value<GuiParentComponent>().parent = 3;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Waves: " + SpawningGroup::waveIndex;
	}
}

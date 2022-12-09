#include "screens.h"
#include "../game.h"

namespace
{
	bool buttonStop(uint32)
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
		return true;
	}
}

void setScreenLost()
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
		Entity *e = ents->create(3);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0.45, 0.05);
		e->value<GuiLayoutLineComponent>().vertical = true;
	}

	{
		Entity *e = ents->create(4);
		e->value<GuiParentComponent>().parent = 3;
		e->value<GuiParentComponent>().order = 0;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "Game Over";
	}

	{
		Entity *e = ents->create(5);
		e->value<GuiParentComponent>().parent = 3;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Waves: " + SpawningGroup::waveIndex;
	}
}

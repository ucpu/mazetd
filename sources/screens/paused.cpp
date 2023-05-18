#include "screens.h"
#include "../game.h"

void setScreenGame();

namespace
{
	bool buttonResume(Entity *)
	{
		setScreenGame();
		return true;
	}

	bool buttonStop(Entity *)
	{
		eventGameReset().dispatch();
		setScreenMainmenu();
		return true;
	}
}

void setScreenPaused()
{
	cleanGui();
	EntityManager *ents = engineGuiEntities();

	{
		Entity *e = ents->create(1);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		e->value<GuiParentComponent>().parent = 1;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Pause";
	}

	{
		Entity *e = ents->create(3);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0.8, 0.666);
	}

	{
		Entity *e = ents->create(4);
		e->value<GuiPanelComponent>();
		e->value<GuiParentComponent>().parent = 3;
		e->value<GuiLayoutLineComponent>().vertical = true;
	}

	{
		Entity *e = ents->create(5);
		e->value<GuiParentComponent>().parent = 4;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "Resume";
		e->value<GuiEventComponent>().event.bind<&buttonResume>();
	}

	{
		Entity *e = ents->create(6);
		e->value<GuiParentComponent>().parent = 4;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "Stop";
		e->value<GuiEventComponent>().event.bind<&buttonStop>();
	}
}

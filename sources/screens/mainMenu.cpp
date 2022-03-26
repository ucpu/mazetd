#include "screens.h"

void setScreenGenerating();
void setScreenAbout();

namespace
{
	bool buttonStart(uint32)
	{
		setScreenGenerating();
		return true;
	}

	bool buttonAbout(uint32)
	{
		setScreenAbout();
		return true;
	}

	bool buttonQuit(uint32)
	{
		engineStop();
		return true;
	}
}

void setScreenMainmenu()
{
	cleanGui();
	EntityManager *ents = engineGuiEntities();

	{
		Entity *e = ents->create(1);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		e->value<GuiParentComponent>().parent = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiTextFormatComponent>().size = 50;
		e->value<GuiTextFormatComponent>().color = Vec3(203, 238, 239) / 255; 
		e->value<GuiTextComponent>().value = "aMAZEing Tower Defense";
	}

	{
		Entity *e = ents->create(3);
		e->value<GuiScrollbarsComponent>().alignment = Vec2(0.8, 0.666);
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
		e->value<GuiTextComponent>().value = "Start";
		e->value<GuiEventComponent>().event.bind<&buttonStart>();
	}

	{
		Entity *e = ents->create(6);
		e->value<GuiParentComponent>().parent = 4;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "About";
		e->value<GuiEventComponent>().event.bind<&buttonAbout>();
	}

	{
		Entity *e = ents->create(7);
		e->value<GuiParentComponent>().parent = 4;
		e->value<GuiParentComponent>().order = 3;
		e->value<GuiButtonComponent>();
		e->value<GuiTextComponent>().value = "Quit";
		e->value<GuiEventComponent>().event.bind<&buttonQuit>();
	}
}

namespace
{
	class Callbacks
	{
		EventListener<void()> engineInitListener;

	public:
		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize, 999);
			engineInitListener.bind<&setScreenMainmenu>();
		}
	} callbacksInstance;
}

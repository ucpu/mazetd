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
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 1;
		GuiLabelComponent &label = e->value<GuiLabelComponent>();
		GuiTextFormatComponent &tf = e->value<GuiTextFormatComponent>();
		tf.size = 50;
		tf.color = Vec3(203, 238, 239) / 255; 
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "aMAZEing Tower Defense";
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
		txt.value = "Start";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonStart>();
	}

	{
		Entity *e = ents->create(6);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 4;
		parent.order = 2;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "About";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonAbout>();
	}

	{
		Entity *e = ents->create(7);
		GuiParentComponent &parent = e->value<GuiParentComponent>();
		parent.parent = 4;
		parent.order = 3;
		GuiButtonComponent &control = e->value<GuiButtonComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Quit";
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonQuit>();
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

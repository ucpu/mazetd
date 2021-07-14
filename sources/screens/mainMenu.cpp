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
	EntityManager *ents = engineGui()->entities();

	{
		Entity *e = ents->create(1);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.45, 0.05);
	}

	{
		Entity *e = ents->create(2);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 1;
		CAGE_COMPONENT_GUI(Label, label, e);
		CAGE_COMPONENT_GUI(TextFormat, tf, e);
		tf.size = 50;
		tf.color = vec3(203, 238, 239) / 255; // #cBEEEF
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "aMAZEing Tower Defense";
	}

	{
		Entity *e = ents->create(3);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.8, 0.666);
	}

	{
		Entity *e = ents->create(4);
		CAGE_COMPONENT_GUI(Panel, panel, e);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 3;
		CAGE_COMPONENT_GUI(LayoutLine, layout, e);
		layout.vertical = true;
	}

	{
		Entity *e = ents->create(5);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		parent.order = 1;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Start";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonStart>();
	}

	{
		Entity *e = ents->create(6);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		parent.order = 2;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "About";
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonAbout>();
	}

	{
		Entity *e = ents->create(7);
		CAGE_COMPONENT_GUI(Parent, parent, e);
		parent.parent = 4;
		parent.order = 3;
		CAGE_COMPONENT_GUI(Button, control, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Quit";
		CAGE_COMPONENT_GUI(Event, ev, e);
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

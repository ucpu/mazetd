#include <cage-core/entities.h>
#include <cage-core/concurrent.h>
#include <cage-engine/engine.h>
#include <cage-engine/gui.h>

using namespace cage;

void screenGame();
void mapGenerate();

namespace
{
	void engineInit();
	void engineUpdate();

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;

	Holder<Thread> mapGenThread;

	void engineInit()
	{
		mapGenThread = newThread(Delegate<void()>().bind<&mapGenerate>(), "map gen");

		EntityManager *ents = engineGui()->entities();

		{
			Entity *e = ents->create(1);
			CAGE_COMPONENT_GUI(Scrollbars, sc, e);
			sc.alignment = vec2(0.5, 0.1);
		}

		{
			Entity *e = ents->create(2);
			CAGE_COMPONENT_GUI(Panel, panel, e);
			CAGE_COMPONENT_GUI(Parent, parent, e);
			parent.parent = 1;
			CAGE_COMPONENT_GUI(LayoutLine, layout, e);
			layout.vertical = true;
		}

		{
			Entity *e = ents->create(3);
			CAGE_COMPONENT_GUI(Parent, parent, e);
			parent.parent = 2;
			CAGE_COMPONENT_GUI(Label, lab, e);
			CAGE_COMPONENT_GUI(Text, txt, e);
			txt.value = "Loading";
		}
	}

	void engineUpdate()
	{
		if (!mapGenThread->done())
			return;

		callbacksInstance.engineUpdateListener.detach();
		mapGenThread.clear();

		engineGui()->skipAllEventsUntilNextUpdate();
		engineGui()->entities()->destroy();
		screenGame();
	}
}

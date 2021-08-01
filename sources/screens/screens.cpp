#include <cage-core/entitiesVisitor.h>

#include "screens.h"

#include <vector>

void cleanGui()
{
	Gui *gui = engineGui();
	gui->skipAllEventsUntilNextUpdate();
	eventGuiClean().dispatch();
	eventGuiClean().detach();
	gui->setFocus(0);
	gui->entities()->destroy();
	gui->widgetEvent.detach();
}

void removeGuiEntitiesWithParent(uint32 parent)
{
	std::vector<uint32> ents;
	entitiesVisitor([&](Entity *e, const GuiParentComponent &p) {
		if (p.parent == parent)
		{
			ents.push_back(e->name());
			e->destroy();
		}
	}, engineGui()->entities(), true);
	for (uint32 n : ents)
		removeGuiEntitiesWithParent(n);
}

EventDispatcher<bool()> &eventGuiClean()
{
	static EventDispatcher<bool()> e;
	return e;
}

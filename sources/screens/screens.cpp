#include <cage-core/entitiesVisitor.h>
#include <cage-engine/guiManager.h>

#include "screens.h"

#include <vector>

void cleanGui()
{
	GuiManager *gui = engineGuiManager();
	gui->invalidateInputs();
	eventGuiClean().dispatch();
	eventGuiClean().detach();
	gui->focus(0);
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
	}, engineGuiEntities(), true);
	for (uint32 n : ents)
		removeGuiEntitiesWithParent(n);
}

EventDispatcher<bool()> &eventGuiClean()
{
	static EventDispatcher<bool()> e;
	return e;
}

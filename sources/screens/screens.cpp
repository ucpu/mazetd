#include <vector>

#include "screens.h"

#include <cage-core/entitiesVisitor.h>
#include <cage-engine/guiManager.h>

namespace mazetd
{
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

	EventDispatcher<bool()> &eventGuiClean()
	{
		static EventDispatcher<bool()> e;
		return e;
	}
}

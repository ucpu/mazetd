#ifndef mazetd_header_screens
#define mazetd_header_screens

#include <cage-core/entities.h>
#include <cage-core/events.h>
#include <cage-engine/guiBuilder.h>
#include <cage-simple/engine.h>

namespace mazetd
{
	using namespace cage;

	void cleanGui();

	EventDispatcher<bool()> &eventGuiClean();

	void setScreenMainmenu();
}

#endif

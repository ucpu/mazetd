#include <cage-core/entities.h>
#include <cage-core/events.h>
#include <cage-engine/guiBuilder.h>
#include <cage-simple/engine.h>

using namespace cage;

void cleanGui();

EventDispatcher<bool()> &eventGuiClean();

void setScreenMainmenu();

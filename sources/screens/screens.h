#include <cage-core/events.h>
#include <cage-core/entities.h>
#include <cage-engine/gui.h>
#include <cage-engine/engine.h>

using namespace cage;

void cleanGui();
void removeGuiEntitiesWithParent(uint32 parent);

EventDispatcher<bool()> &eventGuiClean();

void setScreenMainmenu();

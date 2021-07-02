#include <cage-core/entities.h>

using namespace cage;

struct PositionComponent
{
	uint32 tile = m;
};

struct MovementComponent
{
	uint32 tileStart = m;
	uint32 tileEnd = m;
	uint32 timeStart = m;
	uint32 timeEnd = 0;
};

struct WallComponent
{};

struct MonsterComponent
{
	uint32 visitedWaypointsBits = 0;
	uint32 timeToArrive = 0; // timestamp at which the monster should arrive to the last waypoint
	uint32 life = 0;
};

struct EngineComponent
{
	Entity *entity = nullptr;
};

EntityManager *gameEntities();
EventDispatcher<bool()> &eventGameReset();

extern uint32 gameTime;
extern bool gameRunning;

extern vec3 playerCursorPosition;
extern uint32 playerCursorTile;
extern sint32 playerHealth;
extern uint32 playerMoney;


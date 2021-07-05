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

// player structure that blocks paths
struct BuildingComponent
{};

// player structure that does NOT block path
struct TrapComponent
{};

struct TowerBaseProperties
{
	uint32 damage = 1;
	uint32 splashRadius = 0;
	uint32 firingPeriod = 10;
};

struct TowerComponent : public TowerBaseProperties
{
	uint32 firingDelay = 30;
};

struct MonsterBaseProperties
{
	uint32 damage = 1;
	uint32 life = 1;
	real speed = 0.05;
};

struct MonsterComponent : public MonsterBaseProperties
{
	uint32 visitedWaypointsBits = 0;
	uint32 timeToArrive = 0; // timestamp at which the monster should arrive to the last waypoint
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
extern uint32 playerBuildingSelection;


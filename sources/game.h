#ifndef header_game
#define header_game

#include <cage-core/entities.h>

#include "normalizedReal.h"

enum class DamageTypeEnum
{
	Physical,
	Fire,
	Water,
	Poison,
};

enum class ManaGeneratorTypeEnum
{
	Water,
	Sun,
	Wind,
	Snow,
	Flesh,
};

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

struct NameComponent
{
	string name;
};

// player structure that blocks paths
struct BuildingComponent
{};

// player structure that does NOT block path
struct TrapComponent
{};

struct ManaStorageComponent
{
	uint32 mana = 0;
	uint32 capacity = 1000;
};

struct ManaDistributorComponent
{
	uint32 transferLimit = 10;
};

struct ManaReceiverComponent
{};

struct ManaGeneratorComponent
{
	ManaGeneratorTypeEnum type = ManaGeneratorTypeEnum::Flesh;
};

struct AttackComponent
{
	uint32 damage = 1;
	uint32 firingPeriod = 10;
	uint32 firingDelay = 30;
	uint32 firingRange = rtos32(5);
	uint32 splashRadius = 0;
};

struct AugmentComponent
{
	DamageTypeEnum damageType = DamageTypeEnum::Physical;
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
extern bool playerPanning;

#endif // !header_game

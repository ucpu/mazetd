#ifndef header_game
#define header_game

#include <cage-core/entities.h>

#include "common.h"

enum class DamageTypeEnum
{
	Invalid = 0,
	Physical,
	Fire,
	Water,
	Poison,
	Mana, // not damage but used as type of effect
};

enum class ManaCollectorTypeEnum
{
	Invalid = 0,
	Water,
	Sun,
	Wind,
	Snow,
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

	vec3 position() const;
};

struct PivotComponent
{
	real elevation = 0.5;
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
	uint32 capacity = 20;
};

struct ManaDistributorComponent
{
	uint32 transferLimit = 10;
	real range = 5;
};

struct ManaReceiverComponent
{};

struct ManaCollectorComponent
{
	ManaCollectorTypeEnum type = ManaCollectorTypeEnum::Invalid;
	real range = 5;
};

struct AttackComponent
{
	uint32 firingPeriod = 30;
	uint32 firingDelay = 30;
	real firingRange = 5;
	real splashRadius = 0;
	uint32 damage = 10;
	uint32 damageDuration = 0;
	DamageTypeEnum damageType = DamageTypeEnum::Physical;
	bool useAugments = true;
};

struct AugmentComponent
{
	DamageTypeEnum damageType = DamageTypeEnum::Physical;
	uint32 damageDuration = 10;
};

struct MonsterBaseProperties
{
	uint32 money = 1;
	uint32 damage = 1;
	sint32 life = 100;
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

SpatialQuery *spatialMonsters();
SpatialQuery *spatialStructures();

extern uint32 gameTime;
extern bool gameRunning;

extern vec3 playerCursorPosition;
extern uint32 playerCursorTile;
extern sint32 playerHealth;
extern uint32 playerMoney;
extern uint32 playerBuildingSelection;
extern bool playerPanning;

struct EffectConfig
{
	vec3 pos1 = vec3::Nan();
	vec3 pos2 = vec3::Nan();
	DamageTypeEnum type = DamageTypeEnum::Mana;
};

void renderEffect(const EffectConfig &config);

#endif // !header_game

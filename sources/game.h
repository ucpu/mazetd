#ifndef header_game
#define header_game

#include <cage-core/entities.h>

#include "common.h"

enum class DamageTypeFlags
{
	None = 0,
	Physical = 1u << 0,
	Fire = 1u << 1,
	Water = 1u << 2,
	Poison = 1u << 3,
	Magic = 1u << 4,
	Slow = 1u << 5,
	Haste = 1u << 6,
};

enum class MonsterClassFlags
{
	None = 0,
	Regular = 1u << 0,
	Flyer = 1u << 1,
	Boss = 1u << 2,
};

namespace cage
{
	GCHL_ENUM_BITS(DamageTypeFlags);
	GCHL_ENUM_BITS(MonsterClassFlags);
}

enum class EffectTypeEnum
{
	None = 0,
	Physical,
	Fire,
	Water,
	Poison,
	Mana,
};

enum class ManaCollectorTypeEnum
{
	None = 0,
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
	uint32 capacity = 100;
};

struct ManaDistributorComponent
{
	uint32 transferLimit = 100;
	real range = 5;
};

struct ManaReceiverComponent
{};

struct ManaCollectorComponent
{
	ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
	real range = 7;
	uint32 collectAmount = 1;
};

struct AttackComponent
{
	uint32 firingPeriod = 0;
	uint32 firingDelay = 30;
	real firingRange = 0;
	real splashRadius = 0;
	uint32 damage = 0;
	uint32 manaCost = 0;
	DamageTypeFlags damageType = DamageTypeFlags::None;
	EffectTypeEnum effectType = EffectTypeEnum::None;
	MonsterClassFlags targetClasses = MonsterClassFlags::Regular | MonsterClassFlags::Flyer | MonsterClassFlags::Boss;
	bool useAugments = false;
};

struct AugmentComponent
{
	uint32 damageMultiplier = 2;
	DamageTypeFlags damageType = DamageTypeFlags::None;
	EffectTypeEnum effectType = EffectTypeEnum::None;
};

struct MonsterBaseProperties
{
	StringLiteral name;
	uint32 money = 10;
	uint32 damage = 1;
	sint32 life = 100;
	real speed = 0.05;
	DamageTypeFlags immunities = DamageTypeFlags::None;
	MonsterClassFlags monsterClass = MonsterClassFlags::Regular;
};

struct MonsterComponent : public MonsterBaseProperties
{
	uint32 visitedWaypointsBits = 0;
	uint32 timeToArrive = 0; // timestamp at which the monster should arrive to the last waypoint
};

struct MonsterDebuffComponent
{
	uint32 endTime = 0;
	DamageTypeFlags type = DamageTypeFlags::None;
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
	EffectTypeEnum type = EffectTypeEnum::None;
};

void renderEffect(const EffectConfig &config);

#endif // !header_game

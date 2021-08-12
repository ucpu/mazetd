#ifndef header_game
#define header_game

#include <cage-core/entities.h>

#include "common.h"

// flags & enums

enum class DamageTypeEnum
{
	Physical = 0,
	Fire,
	Water,
	Poison,
	Magic,
	Total,
};

enum class DamageTypeFlags
{
	None = 0,
	Physical = 1u << (uint32)DamageTypeEnum::Physical,
	Fire = 1u << (uint32)DamageTypeEnum::Fire,
	Water = 1u << (uint32)DamageTypeEnum::Water,
	Poison = 1u << (uint32)DamageTypeEnum::Poison,
	Magic = 1u << (uint32)DamageTypeEnum::Magic,
};

enum class MonsterClassFlags
{
	None = 0,
	Regular = 1u << 0,
	Flyer = 1u << 1,
	Boss = 1u << 2,
};

enum class ManaCollectorTypeEnum
{
	None = 0,
	Water,
	Sun,
	Wind,
	Snow,
};

enum class BonusTypeEnum
{
	None = 0,
	Damage,
	FiringRate,
	FiringRange,
	SplashRadius,
	ManaDiscount,
};

enum class TargetingEnum
{
	Random = 0,
	Front, // first monster to finish the whole maze
	Back, // last monster to finish the whole maze
	Closest, // to the tower
	Farthest, // from the tower
	Strongest, // highest life
	Weakest, // lowest life
};

namespace cage
{
	GCHL_ENUM_BITS(DamageTypeFlags);
	GCHL_ENUM_BITS(MonsterClassFlags);
}

// structures

struct MonsterBaseProperties
{
	StringLiteral name;
	uint32 money = 15;
	uint32 damage = 1;
	sint32 life = 50;
	real speed = 0.05;
	DamageTypeFlags immunities = DamageTypeFlags::None;
	MonsterClassFlags monsterClass = MonsterClassFlags::Regular;
};

struct MonsterSpawningProperties : public MonsterBaseProperties
{
	uint32 modelName = 0;
	uint32 animationName = 0;
};

struct SpawningGroup : public MonsterSpawningProperties
{
	uint32 spawnPointsBits = m;
	uint32 spawnCount = 15;
	uint32 spawnSimultaneously = 1;
	uint32 spawnPeriod = 0;
	uint32 spawnDelay = 0;
	bool checkingMonstersCounts = true;

	static inline uint32 waveIndex = 0;

	void spawnOne();
	void process();
	void generate();
	void init();
};

// components

struct EngineComponent
{
	Entity *entity = nullptr;
};

struct GuiModelComponent
{
	uint32 model = 0;
};

struct NameComponent
{
	string name;
};

struct PositionComponent
{
	uint32 tile = m;

	vec3 position() const;
};

struct MovementComponent
{
	uint32 tileStart = m;
	uint32 tileEnd = m;
	uint32 timeStart = m;
	uint32 timeEnd = 0;

	vec3 position() const;
};

struct MonsterComponent : public MonsterBaseProperties
{
	uint32 visitedWaypointsBits = 0;
	uint32 timeToArrive = 0; // timestamp at which the monster should arrive to the final waypoint

	struct DamageOverTime
	{
		uint32 duration = 0;
		uint32 damage = 0;
	};

	DamageOverTime dots[(uint32)DamageTypeEnum::Total];

	bool affected(DamageTypeEnum dmg) const;
};

// player structure that blocks paths
struct BuildingComponent
{};

// player structure that does NOT block path
struct TrapComponent
{};

struct PivotComponent
{
	real elevation = 0.5;
};

struct CostComponent
{
	uint32 cost = 0;
};

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
	uint32 firingDelay = 30;
};

struct DamageComponent
{
	uint32 damage = 0;
	uint32 firingPeriod = 30;
	real firingRange = 5;
	real splashRadius = 0;
	uint32 baseManaCost = 12;
	uint32 baseManaCapacity = 100;
	MonsterClassFlags invalidClasses = MonsterClassFlags::None;
	bool acceptMods = true;
};

struct ModElementComponent
{
	DamageTypeEnum element = DamageTypeEnum::Total;
};

struct ModBonusComponent
{
	BonusTypeEnum type = BonusTypeEnum::None;
};

struct ModTargetingComponent
{
	TargetingEnum targeting = TargetingEnum::Random;
};

// functions & globals

EntityManager *gameEntities();
EventDispatcher<bool()> &eventGameReset();
EventDispatcher<bool()> &eventGameUpdate();

SpatialQuery *spatialMonsters();
SpatialQuery *spatialStructures();

extern uint32 gameTime;
extern uint32 gameSpeed;
extern bool gameRunning;

extern vec3 playerCursorPosition;
extern uint32 playerCursorTile;
extern sint32 playerHealth;
extern sint32 playerMoney;
extern bool playerPanning;
extern Entity *playerBuildingSelection;

extern SpawningGroup spawningGroup;

struct EffectConfig
{
	vec3 pos1 = vec3::Nan();
	vec3 pos2 = vec3::Nan();
	DamageTypeEnum type = DamageTypeEnum::Total;
};

void renderEffect(const EffectConfig &config);

#endif // !header_game

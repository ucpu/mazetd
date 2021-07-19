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

enum class AugmentEnum
{
	None = 0,
	Fire,
	Water,
	Poison,
	Total,
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

struct RefundCostComponent
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

struct AttackData
{
	uint32 firingPeriod = 0;
	real firingRange = 0;
	real splashRadius = 0;
	uint32 damage = 0;
	uint32 manaCost = 0;
	DamageTypeFlags damageType = DamageTypeFlags::None;
	EffectTypeEnum effectType = EffectTypeEnum::None;
	MonsterClassFlags invalidClasses = MonsterClassFlags::None;
};

struct AttackComponent
{
	uint32 firingDelay = 30;
	AttackData data[(int)AugmentEnum::Total];
	bool useAugments = false;

	void initAugmentData();
};

struct AugmentComponent
{
	AugmentEnum data = AugmentEnum::None;
};

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
extern uint32 playerBuildingSelection;
extern bool playerPanning;

extern SpawningGroup spawningGroup;

struct EffectConfig
{
	vec3 pos1 = vec3::Nan();
	vec3 pos2 = vec3::Nan();
	EffectTypeEnum type = EffectTypeEnum::None;
};

void renderEffect(const EffectConfig &config);

#endif // !header_game

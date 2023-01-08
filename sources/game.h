#ifndef header_game
#define header_game

#include <cage-core/entities.h>

#include "common.h"

namespace cage
{
	class SpatialQuery;
}

// flags & enums

enum class DamageTypeEnum
{
	Physical = 0,
	Fire,
	Water,
	Poison,
	Magic,
	Total,
	Mana, // used for renderEffect
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
	Flier = 1u << 0,
	Boss = 1u << 1,
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
	IntenseDot,
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
	StringPointer name;
	sint64 maxLife = 1000;
	uint32 money = 0;
	uint32 damage = 0;
	Real speed = 1; // tiles per second
	DamageTypeFlags resistances = DamageTypeFlags::None;
	MonsterClassFlags monsterClass = MonsterClassFlags::None;
};

struct MonsterSpawningProperties : public MonsterBaseProperties
{
	uint32 modelName = 0;
	uint32 animationName = 0;
};

struct SpawningGroup : public MonsterSpawningProperties
{
	uint32 spawnCount = 0;
	uint32 spawnSimultaneously = 1;
	uint32 spawnPeriod = 15;
	uint32 spawnDelay = 0;
	sint32 bossIndex = m;
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

struct HealthbarComponent
{
	Entity *entity = nullptr;
};

struct ManabarComponent
{
	Entity *entity = nullptr;
};

struct GuiModelComponent
{
	uint32 model = 0;
};

struct NameComponent
{
	StringPointer name;
};

struct DescriptionComponent
{
	StringPointer description;
};

struct PositionComponent
{
	uint32 tile = m;

	Vec3 position() const;
};

struct MovementComponent
{
	uint32 tileStart = m;
	uint32 tileEnd = m;
	uint32 timeStart = m;
	uint32 timeEnd = 0;

	Vec3 position() const;
};

struct MonsterComponent : public MonsterBaseProperties
{
	sint64 life = 0;
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
	Real elevation = 0.5;
};

struct CostComponent
{
	uint32 cost = 0;
};

struct ManaStorageComponent
{
	uint32 mana = 0;
	uint32 capacity = 0;
	sint32 transferOrdering = 0;
};

struct ManaDistributorComponent
{};

struct ManaReceiverComponent
{};

struct ManaCollectorComponent
{
	ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
	Real range = 7;
	uint32 collectAmount = 1;
};

struct AttackComponent
{
	Entity *effectors[3] = {};
	DamageTypeEnum element = DamageTypeEnum::Physical;
	BonusTypeEnum bonus = BonusTypeEnum::None;
	TargetingEnum targeting = TargetingEnum::Random;
	uint32 firingDelay = 30;
};

struct DamageComponent
{
	uint32 damage = 0;
	uint32 overTime = 0;
	uint32 firingPeriod = 30;
	Real firingRange = 5;
	Real splashRadius = 0;
	uint32 manaCost = 12;
	MonsterClassFlags invalidClasses = MonsterClassFlags::None;
	bool acceptMods = true;
};

struct ModElementComponent
{
	DamageTypeEnum element = DamageTypeEnum::Total;
};

struct ModBonusComponent
{
	BonusTypeEnum bonus = BonusTypeEnum::None;
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
extern Real gameSpeed;
extern bool gameReady; // game map has been created
extern bool gamePaused; // player has paused the game to place buildings

extern Vec3 playerCursorPosition;
extern uint32 playerCursorTile;
extern sint32 playerHealth;
extern sint32 playerMoney;
extern bool playerPanning;
extern Entity *playerBuildingSelection;

extern SpawningGroup spawningGroup;

struct EffectConfig
{
	Vec3 pos1 = Vec3::Nan();
	Vec3 pos2 = Vec3::Nan();
	DamageTypeEnum type = DamageTypeEnum::Total;
};

void renderEffect(const EffectConfig &config);

#endif // !header_game

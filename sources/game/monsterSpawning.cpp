#include <cage-core/hashString.h>
#include <cage-core/enumerate.h>
#include <cage-engine/scene.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>
#include <functional>

void updateSpawningMonsterPropertiesScreen();

namespace
{
	std::vector<MonsterSpawningProperties> monsterSpawningProperties;

	uint32 bitsApplyLimit(uint32 bits, uint32 limit)
	{
		for (uint32 i = limit; i < 32; i++)
			bits &= ~(1u << i);
		return bits;
	}

	uint32 bitsPickOneIndex(uint32 bits)
	{
		uint32 cnt = 0;
		for (uint32 i = 0; i < 32; i++)
			cnt += (bits & (1u << i)) > 0;
		uint32 idx = randomRange(0u, cnt);
		for (uint32 i = 0; i < 32; i++)
			if (bits & (1u << i))
				if (idx-- == 0)
					return i;
		CAGE_ASSERT(false);
		return m;
	}

	uint32 bitsLimitRandomOne(uint32 bits)
	{
		return 1u << bitsPickOneIndex(bits);
	}

	uint32 shortestSpawnPointBits()
	{
		return 1u << globalWaypoints->minDistanceSpawner;
	}

	uint32 randomSpawnPointBits()
	{
		uint32 bits = m;
		bits = bitsApplyLimit(bits, numeric_cast<uint32>(globalWaypoints->waypoints.size()));
		bits = bitsLimitRandomOne(bits);
		return bits;
	}

	uint32 allSpawnPointsBits()
	{
		return bitsApplyLimit(m, numeric_cast<uint32>(globalWaypoints->waypoints.size()));
	}

	void generateMonsterProperties()
	{
		monsterSpawningProperties.clear();

		{
			MonsterSpawningProperties msp;
			msp.name = "Frog";
			msp.modelName = HashString("mazetd/monsters/Frog.object");
			msp.animationName = HashString("mazetd/monsters/Frog.glb?Frog_Jump");
			msp.immunities = DamageTypeFlags::Water;
			msp.life *= 0.85;
			msp.speed *= 0.95;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Snake";
			msp.modelName = HashString("mazetd/monsters/Snake.object");
			msp.animationName = HashString("mazetd/monsters/Snake.glb?Snake_Walk");
			msp.immunities = DamageTypeFlags::Poison;
			msp.life *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Spider";
			msp.modelName = HashString("mazetd/monsters/Spider.object");
			msp.animationName = HashString("mazetd/monsters/Spider.glb?Spider_Walk");
			msp.life *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Wasp";
			msp.modelName = HashString("mazetd/monsters/Wasp.object");
			msp.animationName = HashString("mazetd/monsters/Wasp.glb?Wasp_Flying");
			msp.immunities = DamageTypeFlags::Poison;
			msp.monsterClass = MonsterClassFlags::Flyer;
			msp.life *= 0.85;
			msp.speed *= 1.05;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Stegosaurus";
			msp.modelName = HashString("mazetd/monsters/Stegosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Stegosaurus.glb?Stegosaurus_Walk");
			msp.immunities = DamageTypeFlags::Fire;
			msp.life *= 1.25;
			msp.speed *= 0.95;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Rat";
			msp.modelName = HashString("mazetd/monsters/Rat.object");
			msp.animationName = HashString("mazetd/monsters/Rat.glb?Rat_Walk");
			msp.immunities = DamageTypeFlags::Water;
			msp.life *= 0.9;
			msp.speed *= 1.1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Velociraptor";
			msp.modelName = HashString("mazetd/monsters/Velociraptor.object");
			msp.animationName = HashString("mazetd/monsters/Velociraptor.glb?Velociraptor_Walk");
			msp.life *= 1.25;
			msp.speed *= 1.25;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Slime";
			msp.modelName = HashString("mazetd/monsters/Slime.object");
			msp.animationName = HashString("mazetd/monsters/Slime.glb?Slime_Idle");
			msp.immunities = DamageTypeFlags::Fire | DamageTypeFlags::Magic;
			msp.life *= 1;
			msp.speed *= 0.8;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Triceratops";
			msp.modelName = HashString("mazetd/monsters/Triceratops.object");
			msp.animationName = HashString("mazetd/monsters/Triceratops.glb?Triceratops_Walk");
			msp.immunities = DamageTypeFlags::Poison;
			msp.life *= 1.25;
			msp.speed *= 1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Bat";
			msp.modelName = HashString("mazetd/monsters/Bat.object");
			msp.animationName = HashString("mazetd/monsters/Bat.glb?Bat_Flying");
			msp.monsterClass = MonsterClassFlags::Flyer;
			msp.life *= 1;
			msp.speed *= 0.95;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Apatosaurus";
			msp.modelName = HashString("mazetd/monsters/Apatosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Apatosaurus.glb?Apatosaurus_Walk");
			msp.immunities = DamageTypeFlags::Physical;
			msp.life *= 1.5;
			msp.speed *= 0.8;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Skeleton";
			msp.modelName = HashString("mazetd/monsters/Skeleton.object");
			msp.animationName = HashString("mazetd/monsters/Skeleton.glb?Skeleton_Running");
			msp.immunities = DamageTypeFlags::Physical | DamageTypeFlags::Magic;
			msp.life *= 1.15;
			msp.speed *= 0.95;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Parasaurolophus";
			msp.modelName = HashString("mazetd/monsters/Parasaurolophus.object");
			msp.animationName = HashString("mazetd/monsters/Parasaurolophus.glb?Parasaurolophus_Walk");
			msp.immunities = DamageTypeFlags::Water;
			msp.life *= 1.25;
			msp.speed *= 1.1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Trex";
			msp.modelName = HashString("mazetd/monsters/Trex.object");
			msp.animationName = HashString("mazetd/monsters/Trex.glb?TRex_Walk");
			msp.immunities = DamageTypeFlags::Physical;
			msp.life *= 1.25;
			msp.speed *= 1.05;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Dragon";
			msp.modelName = HashString("mazetd/monsters/Dragon.object");
			msp.animationName = HashString("mazetd/monsters/Dragon.glb?Dragon_Flying");
			msp.immunities = DamageTypeFlags::Fire | DamageTypeFlags::Magic;
			msp.monsterClass = MonsterClassFlags::Flyer;
			msp.life *= 1.5;
			msp.speed *= 1.15;
			monsterSpawningProperties.push_back(msp);
		}
	}

	void gameReset()
	{
		generateMonsterProperties();
		spawningGroup.init();
	}

	void gameUpdate()
	{
		spawningGroup.process();
	}

	struct Callbacks
	{
		EventListener<void()> gameResetListener;
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

void SpawningGroup::spawnOne()
{
	Entity *e = gameEntities()->createUnique();

	const uint32 spawnPointIndex = bitsPickOneIndex(spawnPointsBits);
	const uint32 position = globalWaypoints->waypoints[spawnPointIndex]->tile;
	e->value<PositionComponent>().tile = position;
	e->value<NameComponent>().name = name;

	MonsterComponent &mo = e->value<MonsterComponent>();
	(MonsterBaseProperties &)mo = (const MonsterBaseProperties &)*this;
	mo.visitedWaypointsBits = 1u << spawnPointIndex;
	mo.timeToArrive = gameTime + numeric_cast<uint32>(30 * stor(globalWaypoints->find(position, mo.visitedWaypointsBits).distance) / mo.speed);

	MovementComponent &mv = e->value<MovementComponent>();
	mv.tileStart = mv.tileEnd = position;
	mv.timeStart = mv.timeEnd = gameTime;

	Entity *f = e->value<EngineComponent>().entity;
	RenderComponent &r = f->value<RenderComponent>();
	r.object = modelName;
	SkeletalAnimationComponent &a = f->value<SkeletalAnimationComponent>();
	a.name = animationName;
	a.offset = randomRange(0.f, 1e6f);

	if (bossIndex-- == 0)
	{
		mo.monsterClass |= MonsterClassFlags::Boss;
		mo.life *= 2;
		f->value<TransformComponent>().scale = 1.5;
	}
}

void SpawningGroup::process()
{
	if (spawnDelay > 0)
	{
		spawnDelay--;
		return;
	}

	if (checkingMonstersCounts && gameEntities()->component<MonsterComponent>()->count() > waveIndex / 3)
		return;

	CAGE_ASSERT(spawnCount > 0);
	spawnCount--;
	spawnDelay += spawnPeriod;
	checkingMonstersCounts = false;

	for (uint32 simultaneously = 0; simultaneously < spawnSimultaneously; simultaneously++)
		spawnOne();

	if (spawnCount == 0)
		generate();
}

void SpawningGroup::generate()
{
	// assumed balance estimation:
	// map has 3000 playable tiles
	// maximum shortest path is 2250 tiles long (75 % of playable tiles)
	// naive build has 0.25 dps per dollar (using inefficient towers)
	// simple build has 0.5 dps per dollar (using efficient towers but no mana)
	// intermediate build has 1 dps per dollar (using mana but not combining elements)
	// optimized build has 2 dps per dollar (using mana and efficiently combining elements)
	// collecting maximum 150 mana per second
	// estimated 50'000 dps using elements+magic in optimized build costing 30'000 money (consumes 400 mana per second)
	// estimated 50'000 dps using physical+poison in optimized build costing 45'000 money
	// 2000 walls costing 10'000 money

	const uint32 monsterVarietes = numeric_cast<uint32>(monsterSpawningProperties.size());
	const uint32 totalWaves = monsterVarietes * 3;
	const Real normWave = pow(saturate(waveIndex / Real(totalWaves - 1)), 1.5);

	*this = {};
	const MonsterSpawningProperties &proto = monsterSpawningProperties[waveIndex % monsterVarietes];
	(MonsterSpawningProperties &)*this = proto;

	if (waveIndex < monsterVarietes)
		immunities = DamageTypeFlags::None;
	spawnPointsBits = shortestSpawnPointBits();
	spawnCount = interpolate(30, 25, normWave);
	bossIndex = randomRange(0u, spawnCount);
	money = interpolate(500, 5'000, normWave) / spawnCount; // total of 100'000 for 45 waves
	damage = interpolate(3, 10, normWave);
	speed = interpolate(2, 5, normWave) * speed; // 5 minutes for 1500 tiles
	life = interpolate((sint64)1'000, (sint64)1'000'000, pow(waveIndex / Real(totalWaves - 1), 1.7)) * life / 1000 / spawnCount;

	waveIndex++;
	updateSpawningMonsterPropertiesScreen();
}

void SpawningGroup::init()
{
	*this = {};
	spawnCount = 1;
	spawnSimultaneously = 0;
	spawnDelay = 150;
	waveIndex = 0;
}

SpawningGroup spawningGroup;

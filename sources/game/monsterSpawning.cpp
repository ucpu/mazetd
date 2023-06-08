#include <cage-core/enumerate.h>
#include <cage-core/hashString.h>
#include <cage-engine/scene.h>

#include "../game.h"
#include "../grid.h"

#include <algorithm>
#include <functional>
#include <vector>

void setScreenWon();
void updateSpawningMonsterPropertiesScreen();

namespace
{
	std::vector<MonsterSpawningProperties> monsterSpawningProperties;

	void generateMonsterProperties()
	{
		monsterSpawningProperties.clear();

		{
			MonsterSpawningProperties msp;
			msp.name = "Frog";
			msp.modelName = HashString("mazetd/monsters/Frog.object");
			msp.animationName = HashString("mazetd/monsters/Frog.glb?Frog_Jump");
			msp.resistances = DamageTypeFlags::Water;
			msp.maxLife *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Snake";
			msp.modelName = HashString("mazetd/monsters/Snake.object");
			msp.animationName = HashString("mazetd/monsters/Snake.glb?Snake_Walk");
			msp.resistances = DamageTypeFlags::Poison;
			msp.maxLife *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Spider";
			msp.modelName = HashString("mazetd/monsters/Spider.object");
			msp.animationName = HashString("mazetd/monsters/Spider.glb?Spider_Walk");
			msp.maxLife *= 1;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Wasp";
			msp.modelName = HashString("mazetd/monsters/Wasp.object");
			msp.animationName = HashString("mazetd/monsters/Wasp.glb?Wasp_Flying");
			msp.resistances = DamageTypeFlags::Poison;
			msp.monsterClass = MonsterClassFlags::Flier;
			msp.maxLife *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Stegosaurus";
			msp.modelName = HashString("mazetd/monsters/Stegosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Stegosaurus.glb?Stegosaurus_Walk");
			msp.resistances = DamageTypeFlags::Fire;
			msp.maxLife *= 1.1;
			msp.speed *= 1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Rat";
			msp.modelName = HashString("mazetd/monsters/Rat.object");
			msp.animationName = HashString("mazetd/monsters/Rat.glb?Rat_Walk");
			msp.resistances = DamageTypeFlags::Water;
			msp.maxLife *= 0.9;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Velociraptor";
			msp.modelName = HashString("mazetd/monsters/Velociraptor.object");
			msp.animationName = HashString("mazetd/monsters/Velociraptor.glb?Velociraptor_Walk");
			msp.maxLife *= 1.2;
			msp.speed *= 1.15;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Slime";
			msp.modelName = HashString("mazetd/monsters/Slime.object");
			msp.animationName = HashString("mazetd/monsters/Slime.glb?Slime_Idle");
			msp.resistances = DamageTypeFlags::Fire | DamageTypeFlags::Magic;
			msp.maxLife *= 0.8;
			msp.speed *= 0.8;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Triceratops";
			msp.modelName = HashString("mazetd/monsters/Triceratops.object");
			msp.animationName = HashString("mazetd/monsters/Triceratops.glb?Triceratops_Walk");
			msp.resistances = DamageTypeFlags::Poison;
			msp.maxLife *= 1.1;
			msp.speed *= 1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Bat";
			msp.modelName = HashString("mazetd/monsters/Bat.object");
			msp.animationName = HashString("mazetd/monsters/Bat.glb?Bat_Flying");
			msp.monsterClass = MonsterClassFlags::Flier;
			msp.maxLife *= 1;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Apatosaurus";
			msp.modelName = HashString("mazetd/monsters/Apatosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Apatosaurus.glb?Apatosaurus_Walk");
			msp.resistances = DamageTypeFlags::Physical;
			msp.maxLife *= 1.1;
			msp.speed *= 0.9;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Skeleton";
			msp.modelName = HashString("mazetd/monsters/Skeleton.object");
			msp.animationName = HashString("mazetd/monsters/Skeleton.glb?Skeleton_Running");
			msp.resistances = DamageTypeFlags::Physical | DamageTypeFlags::Magic;
			msp.maxLife *= 0.8;
			msp.speed *= 0.8;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Parasaurolophus";
			msp.modelName = HashString("mazetd/monsters/Parasaurolophus.object");
			msp.animationName = HashString("mazetd/monsters/Parasaurolophus.glb?Parasaurolophus_Walk");
			msp.resistances = DamageTypeFlags::Water;
			msp.maxLife *= 1.1;
			msp.speed *= 1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Trex";
			msp.modelName = HashString("mazetd/monsters/Trex.object");
			msp.animationName = HashString("mazetd/monsters/Trex.glb?TRex_Walk");
			msp.resistances = DamageTypeFlags::Physical;
			msp.maxLife *= 1.1;
			msp.speed *= 1.1;
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Dragon";
			msp.modelName = HashString("mazetd/monsters/Dragon.object");
			msp.animationName = HashString("mazetd/monsters/Dragon.glb?Dragon_Flying");
			msp.resistances = DamageTypeFlags::Fire | DamageTypeFlags::Magic;
			msp.monsterClass = MonsterClassFlags::Flier;
			msp.maxLife *= 1;
			msp.speed *= 1.2;
			monsterSpawningProperties.push_back(msp);
		}
	}

	const auto gameResetListener = eventGameReset().listen(
		[]()
		{
			generateMonsterProperties();
			spawningGroup.init();
		});

	const auto gameUpdateListener = eventGameUpdate().listen([]() { spawningGroup.process(); });
}

void SpawningGroup::spawnOne()
{
	Entity *e = gameEntities()->createUnique();

	const uint32 position = globalWaypoints->waypoints[globalWaypoints->minDistanceSpawner]->tile;
	e->value<PositionComponent>().tile = position;
	e->value<NameComponent>().name = name;

	MonsterComponent &mo = e->value<MonsterComponent>();
	(MonsterBaseProperties &)mo = (const MonsterBaseProperties &)*this;
	mo.visitedWaypointsBits = 1u << globalWaypoints->minDistanceSpawner;
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
		mo.maxLife *= 2;
		f->value<TransformComponent>().scale = 1.5;
	}

	mo.life = mo.maxLife;
	e->value<HealthbarComponent>();
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
	spawnDelay += spawnPeriod + randomRange(-5, 5);
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
	// collecting maximum 100 mana per second
	// estimated 50'000 dps using elements+magic in optimized build costing 30'000 money (consumes 400 mana per second)
	// estimated 50'000 dps using physical+poison in optimized build costing 45'000 money
	// 1500 walls costing 6'000 money

	const uint32 monsterVarietes = numeric_cast<uint32>(monsterSpawningProperties.size());
	const uint32 totalWaves = monsterVarietes * 3;
	const Real normWave = pow(saturate(waveIndex / Real(totalWaves - 1)), 1.5);

	*this = {};
	const MonsterSpawningProperties &proto = monsterSpawningProperties[waveIndex % monsterVarietes];
	(MonsterSpawningProperties &)*this = proto;

	if (waveIndex < monsterVarietes)
		resistances = DamageTypeFlags::None;
	spawnCount = interpolate(20, 25, normWave);
	bossIndex = randomRange(0u, spawnCount);
	money = interpolate(500, 2'500, normWave) / spawnCount; // total of 60'000 for 45 waves (+ some money per time)
	damage = interpolate(3, 5, normWave);
	speed = interpolate(2, 5, normWave) * speed; // 5 minutes for 1500 tiles
	maxLife = interpolate((sint64)3'000, (sint64)1'500'000, pow(waveIndex / Real(totalWaves - 1), 2)) * maxLife / 1000 / spawnCount;

	waveIndex++;
	updateSpawningMonsterPropertiesScreen();

	if (waveIndex == totalWaves + 1)
		setScreenWon();
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

#include <cage-core/hashString.h>
#include <cage-core/enumerate.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>

namespace
{
	struct MonsterSpawningProperties : public MonsterBaseProperties
	{
		StringLiteral name;
		uint32 modelName = 0;
		uint32 animationName = 0;
	};

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

	struct SpawningGroup
	{
		StringLiteral name;
		
		uint32 monstersBits = 0;
		bool pickOneMonsterType = false;

		uint32 spawnPointsBits = m;
		bool pickOneSpawnPoint = false;

		uint32 spawnRounds = 1;
		uint32 spawnSimultaneously = 1;
		uint32 spawnPeriod = 5;
		uint32 spawnDelay = 30;

		double order = 0;
		double orderAddition = 1;
		double orderFactor = 1;

		void updateOrder()
		{
			order += orderAddition;
			orderAddition *= orderFactor;
		}

		void prepare()
		{
			monstersBits = bitsApplyLimit(monstersBits, numeric_cast<uint32>(monsterSpawningProperties.size()));
			spawnPointsBits = bitsApplyLimit(spawnPointsBits, numeric_cast<uint32>(globalWaypoints->waypoints.size()));
			if (pickOneMonsterType)
				monstersBits = bitsLimitRandomOne(monstersBits);
			if (pickOneSpawnPoint)
				spawnPointsBits = bitsLimitRandomOne(spawnPointsBits);
		}

		void spawnOne()
		{
			Entity *e = gameEntities()->createUnique();

			const uint32 spawnPointIndex = bitsPickOneIndex(spawnPointsBits);
			const uint32 position = globalWaypoints->waypoints[spawnPointIndex]->tile;
			e->value<PositionComponent>().tile = position;

			const uint32 monsterSpawningPropsIndex = bitsPickOneIndex(monstersBits);
			const MonsterSpawningProperties &msp = monsterSpawningProperties[monsterSpawningPropsIndex];
			MonsterComponent &mo = e->value<MonsterComponent>();
			(MonsterBaseProperties &)mo = (const MonsterBaseProperties &)msp;

			mo.visitedWaypointsBits = 1u << spawnPointIndex;
			mo.timeToArrive = gameTime + numeric_cast<uint32>(stor(globalWaypoints->find(position, mo.visitedWaypointsBits).distance) / mo.speed);

			MovementComponent &mv = e->value<MovementComponent>();
			mv.tileStart = mv.tileEnd = position;
			mv.timeStart = mv.timeEnd = gameTime;

			Entity *f = e->value<EngineComponent>().entity;
			CAGE_COMPONENT_ENGINE(Render, r, f);
			r.object = msp.modelName;
			CAGE_COMPONENT_ENGINE(SkeletalAnimation, a, f);
			a.name = msp.animationName;
			a.offset = randomRange(0.f, 1e6f);
		}

		void process()
		{
			if (spawnRounds == 0)
				return;
			if (spawnDelay > 0)
			{
				spawnDelay--;
				return;
			}
			spawnDelay += spawnPeriod;
			spawnRounds--;

			for (uint32 simultaneously = 0; simultaneously < spawnSimultaneously; simultaneously++)
				spawnOne();
		}
	};

	std::vector<SpawningGroup> spawningGroups;
	SpawningGroup spawning;

	void engineUpdate()
	{
		if (!globalGrid || !gameRunning)
			return;

		spawning.process();
		if (spawning.spawnRounds > 0)
			return;

#ifdef CAGE_DEBUG
		constexpr const uint32 monsterLimit = 50;
#else
		constexpr const uint32 monsterLimit = 100;
#endif // CAGE_DEBUG
		if (gameEntities()->component<MonsterComponent>()->count() >= monsterLimit)
			return;

		spawning = spawningGroups.front();
		spawning.prepare();
		spawningGroups.front().updateOrder();
		std::sort(spawningGroups.begin(), spawningGroups.end(), [](const SpawningGroup &a, const SpawningGroup &b) {
			return a.order < b.order;
		});
	}

	void generateMonsterProperties()
	{
		{
			MonsterSpawningProperties msp;
			msp.name = "Frog";
			msp.modelName = HashString("mazetd/monsters/Frog.object");
			msp.animationName = HashString("mazetd/monsters/Frog.glb?Frog_Jump");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Rat";
			msp.modelName = HashString("mazetd/monsters/Rat.object");
			msp.animationName = HashString("mazetd/monsters/Rat.glb?Rat_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Snake";
			msp.modelName = HashString("mazetd/monsters/Snake.object");
			msp.animationName = HashString("mazetd/monsters/Snake.glb?Snake_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Spider";
			msp.modelName = HashString("mazetd/monsters/Spider.object");
			msp.animationName = HashString("mazetd/monsters/Spider.glb?Spider_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Wasp";
			msp.modelName = HashString("mazetd/monsters/Wasp.object");
			msp.animationName = HashString("mazetd/monsters/Wasp.glb?Wasp_Flying");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Bat";
			msp.modelName = HashString("mazetd/monsters/Bat.object");
			msp.animationName = HashString("mazetd/monsters/Bat.glb?Bat_Flying");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Dragon";
			msp.modelName = HashString("mazetd/monsters/Dragon.object");
			msp.animationName = HashString("mazetd/monsters/Dragon.glb?Dragon_Flying");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Skeleton";
			msp.modelName = HashString("mazetd/monsters/Skeleton.object");
			msp.animationName = HashString("mazetd/monsters/Skeleton.glb?Skeleton_Running");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Slime";
			msp.modelName = HashString("mazetd/monsters/Slime.object");
			msp.animationName = HashString("mazetd/monsters/Slime.glb?Slime_Idle");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Apatosaurus";
			msp.modelName = HashString("mazetd/monsters/Apatosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Apatosaurus.glb?Apatosaurus_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Parasaurolophus";
			msp.modelName = HashString("mazetd/monsters/Parasaurolophus.object");
			msp.animationName = HashString("mazetd/monsters/Parasaurolophus.glb?Parasaurolophus_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Stegosaurus";
			msp.modelName = HashString("mazetd/monsters/Stegosaurus.object");
			msp.animationName = HashString("mazetd/monsters/Stegosaurus.glb?Stegosaurus_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Trex";
			msp.modelName = HashString("mazetd/monsters/Trex.object");
			msp.animationName = HashString("mazetd/monsters/Trex.glb?TRex_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Triceratops";
			msp.modelName = HashString("mazetd/monsters/Triceratops.object");
			msp.animationName = HashString("mazetd/monsters/Triceratops.glb?Triceratops_Walk");
			monsterSpawningProperties.push_back(msp);
		}

		{
			MonsterSpawningProperties msp;
			msp.name = "Velociraptor";
			msp.modelName = HashString("mazetd/monsters/Velociraptor.object");
			msp.animationName = HashString("mazetd/monsters/Velociraptor.glb?Velociraptor_Walk");
			monsterSpawningProperties.push_back(msp);
		}
	}

	void generateSpawningGroups()
	{
		for (const auto &it : enumerate(monsterSpawningProperties))
		{
			SpawningGroup sg;
			sg.name = it->name;
			sg.monstersBits = 1u << it.index;
			spawningGroups.push_back(sg);
		}
	}

	void gameReset()
	{
		monsterSpawningProperties.clear();
		spawningGroups.clear();
		spawning = {};
		spawning.spawnRounds = 0;
		generateMonsterProperties();
		generateSpawningGroups();
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;
		EventListener<void()> gameResetListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
		}
	} callbacksInstance;
}

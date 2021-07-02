#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <bitset>

namespace
{
	constexpr const uint32 monsterModels[] = {
		HashString("mazetd/monsters/Frog.object"),
		HashString("mazetd/monsters/Rat.object"),
		HashString("mazetd/monsters/Snake.object"),
		HashString("mazetd/monsters/Spider.object"),
		HashString("mazetd/monsters/Wasp.object"),
		HashString("mazetd/monsters/Bat.object"),
		HashString("mazetd/monsters/Dragon.object"),
		HashString("mazetd/monsters/Skeleton.object"),
		HashString("mazetd/monsters/Slime.object"),
		HashString("mazetd/monsters/Apatosaurus.object"),
		HashString("mazetd/monsters/Parasaurolophus.object"),
		HashString("mazetd/monsters/Stegosaurus.object"),
		HashString("mazetd/monsters/Trex.object"),
		HashString("mazetd/monsters/Triceratops.object"),
		HashString("mazetd/monsters/Velociraptor.object"),
	};

	constexpr const uint32 monsterAnimations[] = {
		HashString("mazetd/monsters/Frog.glb?Frog_Jump"),
		HashString("mazetd/monsters/Rat.glb?Rat_Walk"),
		HashString("mazetd/monsters/Snake.glb?Snake_Walk"),
		HashString("mazetd/monsters/Spider.glb?Spider_Walk"),
		HashString("mazetd/monsters/Wasp.glb?Wasp_Flying"),
		HashString("mazetd/monsters/Bat.glb?Bat_Flying"),
		HashString("mazetd/monsters/Dragon.glb?Dragon_Flying"),
		HashString("mazetd/monsters/Skeleton.glb?Skeleton_Running"),
		HashString("mazetd/monsters/Slime.glb?Slime_Idle"),
		HashString("mazetd/monsters/Apatosaurus.glb?Apatosaurus_Walk"),
		HashString("mazetd/monsters/Parasaurolophus.glb?Parasaurolophus_Walk"),
		HashString("mazetd/monsters/Stegosaurus.glb?Stegosaurus_Walk"),
		HashString("mazetd/monsters/Trex.glb?TRex_Walk"),
		HashString("mazetd/monsters/Triceratops.glb?Triceratops_Walk"),
		HashString("mazetd/monsters/Velociraptor.glb?Velociraptor_Walk"),
	};

	static_assert(sizeof(monsterModels) == sizeof(monsterAnimations));

	void spawnMonsters()
	{
		Entity *e = gameEntities()->createUnique();
		const uint32 spawnIndex = randomRange(0u, numeric_cast<uint32>(globalWaypoints->waypoints.size()));
		const uint32 position = globalWaypoints->waypoints[spawnIndex]->tile;
		e->value<PositionComponent>().tile = position;
		MonsterComponent &mo = e->value<MonsterComponent>();
		mo.visitedWaypointsBits = 1u << spawnIndex;
		mo.timeToArrive = gameTime +  globalWaypoints->find(position, mo.visitedWaypointsBits).distance / 10;
		MovementComponent &mv = e->value<MovementComponent>();
		mv.tileStart = mv.tileEnd = position;
		mv.timeStart = mv.timeEnd = gameTime;
		Entity *f = e->value<EngineComponent>().entity;
		const uint32 type = randomRange(0u, (uint32)(sizeof(monsterModels) / sizeof(monsterModels[0])));
		CAGE_COMPONENT_ENGINE(Render, r, f);
		r.object = monsterModels[type];
		CAGE_COMPONENT_ENGINE(SkeletalAnimation, a, f);
		a.name = monsterAnimations[type];
		a.offset = randomRange(0.f, 1e6f);
	}

	uint32 bitCount(uint32 v)
	{
		return std::bitset<32>(v).count();
	}

	void moveMonsters()
	{
		const uint32 time = gameTime;
		entitiesVisitor(gameEntities(), [&](Entity *e, PositionComponent &po, MovementComponent &mv, MonsterComponent &mo) {
			CAGE_ASSERT(po.tile == mv.tileEnd);
			if (time < mv.timeEnd)
				return;
			for (uint32 i = 0; i < globalWaypoints->waypoints.size(); i++)
				if (globalWaypoints->waypoints[i]->tile == po.tile)
					mo.visitedWaypointsBits |= 1u << i;
			if (bitCount(mo.visitedWaypointsBits) == globalWaypoints->waypoints.size())
			{
				// the monster has reached its final waypoint
				e->destroy();
				return;
			}
			const auto go = globalWaypoints->find(po.tile, mo.visitedWaypointsBits);
			CAGE_ASSERT(go.tile != m);
			CAGE_ASSERT(go.distance > 0);
			mv.tileStart = po.tile;
			mv.tileEnd = po.tile = go.tile;
			mv.timeStart = time;
			const uint32 distNext = globalGrid->neighborDistance(mv.tileStart, mv.tileEnd);
			const uint32 timeAvail = mo.timeToArrive >= time ? mo.timeToArrive - time : 0;
			mv.timeEnd = time + timeAvail * distNext / go.distance;
		}, true);
	}

	void engineUpdate()
	{
		if (!gameRunning || !globalGrid)
			return;
		if (gameTime % 30 == 0 && gameEntities()->component<MonsterComponent>()->count() < 50)
			spawnMonsters();
		moveMonsters();
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <bitset>

namespace
{
	void spawnMonsters()
	{
		if (gameTime() % 30 != 0 || gameEntities()->component<MonsterComponent>()->count() >= 50)
			return;
		Entity *e = gameEntities()->createUnique();
		const uint32 spawnIndex = randomRange(0u, numeric_cast<uint32>(globalPaths->paths.size()));
		const uint32 position = globalPaths->paths[spawnIndex]->tile;
		e->value<PositionComponent>().tile = position;
		MonsterComponent &mo = e->value<MonsterComponent>();
		mo.visitedWaypointsBits = 1u << spawnIndex;
		mo.timeToArrive = gameTime() +  globalPaths->find(position, mo.visitedWaypointsBits).distance / 10;
		MovementComponent &mv = e->value<MovementComponent>();
		mv.tileStart = mv.tileEnd = position;
		mv.timeStart = mv.timeEnd = gameTime();
		e->value<EngineComponent>();
	}

	uint32 bitCount(uint32 v)
	{
		return std::bitset<32>(v).count();
	}

	void moveMonsters()
	{
		const uint32 time = gameTime();
		entitiesVisitor(gameEntities(), [&](Entity *e, PositionComponent &po, MovementComponent &mv, MonsterComponent &mo) {
			CAGE_ASSERT(po.tile == mv.tileEnd);
			if (time < mv.timeEnd)
				return;
			for (uint32 i = 0; i < globalPaths->paths.size(); i++)
				if (globalPaths->paths[i]->tile == po.tile)
					mo.visitedWaypointsBits |= 1u << i;
			if (bitCount(mo.visitedWaypointsBits) == globalPaths->paths.size())
			{
				// the monster has reached its final waypoint
				e->destroy();
				return;
			}
			const auto go = globalPaths->find(po.tile, mo.visitedWaypointsBits);
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
		if (!globalGrid)
			return;
		//globalPaths->update();
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

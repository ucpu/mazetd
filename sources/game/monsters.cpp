#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <bitset>

namespace
{
	uint32 bitCount(uint32 v)
	{
		return std::bitset<32>(v).count();
	}

	void updateMonsterAnimations()
	{
		EntityComponent *aniComp = engineEntities()->component<SkeletalAnimationComponent>();
		if (gameRunning)
		{
			entitiesVisitor(gameEntities(), [&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const EngineComponent &en) {
				Entity *f = en.entity;
				if (!f->has(aniComp))
					return;
				SkeletalAnimationComponent &a = f->value<SkeletalAnimationComponent>(aniComp);
				const uint32 moveDur = mv.timeEnd - mv.timeStart;
				const real dist = stor(globalGrid->neighborDistance(mv.tileStart, mv.tileEnd));
				const real moveSpeed = dist / moveDur;
				a.speed = moveSpeed / mo.speed;
			});
		}
		else
		{
			entitiesVisitor(gameEntities(), [&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const EngineComponent &en) {
				Entity *f = en.entity;
				if (!f->has(aniComp))
					return;
				SkeletalAnimationComponent &a = f->value<SkeletalAnimationComponent>(aniComp);
				a.speed = 0;
			});
		}
	}

	void killMonsters()
	{
		entitiesVisitor(gameEntities(), [&](Entity *e, const MonsterComponent &mo) {
			if (mo.life <= 0)
			{
				playerMoney += mo.money;
				e->destroy();
			}
		}, true);
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
				playerHealth -= mo.damage;
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
		if (!globalGrid)
			return;
		updateMonsterAnimations();
		if (!gameRunning)
			return;
		killMonsters();
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

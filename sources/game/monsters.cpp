#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <bitset>

uint32 bitCount(uint32 v)
{
	return std::bitset<32>(v).count();
}

void createMonsterGhost(Entity *e);

namespace
{
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
				createMonsterGhost(e);
				e->destroy();
			}
		}, true);
	}

	void moveMonsters()
	{
		EntityComponent *debComp = gameEntities()->component<MonsterDebuffComponent>();
		entitiesVisitor(gameEntities(), [&](Entity *e, PositionComponent &po, MovementComponent &mv, MonsterComponent &mo) {
			CAGE_ASSERT(po.tile == mv.tileEnd);
			if (gameTime < mv.timeEnd)
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
			mv.timeStart = gameTime;
			const uint32 distNext = globalGrid->neighborDistance(mv.tileStart, mv.tileEnd);
			const uint32 timeAvail = mo.timeToArrive >= gameTime ? mo.timeToArrive - gameTime : 0;
			mv.timeEnd = gameTime + timeAvail * distNext / go.distance;

			if (e->has(debComp))
			{
				const MonsterDebuffComponent &deb = e->value<MonsterDebuffComponent>(debComp);
				if (any(deb.type & DamageTypeFlags::Slow) && none(deb.type & DamageTypeFlags::Haste))
					mv.timeEnd += 3 * (mv.timeEnd - mv.timeStart) / 2;
				if (none(deb.type & DamageTypeFlags::Slow) && any(deb.type & DamageTypeFlags::Haste))
					mv.timeEnd -= 1 * (mv.timeEnd - mv.timeStart) / 2;
			}
		}, true);
	}

	void updateDebuffs()
	{
		EntityComponent *debComp = gameEntities()->component<MonsterDebuffComponent>();
		entitiesVisitor(gameEntities(), [&](Entity *e, const MonsterDebuffComponent &deb) {
			if (gameTime >= deb.endTime)
				e->remove(debComp);
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
		updateDebuffs();
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

#include <cage-core/entitiesVisitor.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <bitset>

vec3 MovementComponent::position() const
{
	const vec3 ca = globalGrid->center(tileStart);
	const vec3 cb = globalGrid->center(tileEnd);
	const real fac = saturate(real(gameTime - (sint64)timeStart) / real(timeEnd - (sint64)timeStart));
	return interpolate(ca, cb, fac);
}

bool MonsterComponent::affected(DamageTypeEnum dmg) const
{
	return dots[(uint32)dmg].duration > 0;
}

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
		const uint32 speed = gameRunning ? gameSpeed : 0;
		entitiesVisitor([&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const EngineComponent &en) {
			Entity *f = en.entity;
			if (!f->has(aniComp))
				return;
			SkeletalAnimationComponent &a = f->value<SkeletalAnimationComponent>(aniComp);
			const uint32 moveDur = mv.timeEnd - mv.timeStart;
			const real dist = stor(globalGrid->neighborDistance(mv.tileStart, mv.tileEnd));
			const real moveSpeed = dist / moveDur;
			a.speed = speed * moveSpeed / mo.speed;
		}, gameEntities(), false);
	}

	void killMonsters()
	{
		entitiesVisitor([&](Entity *e, const MonsterComponent &mo) {
			if (mo.life <= 0)
			{
				playerMoney += mo.money;
				createMonsterGhost(e);
				e->destroy();
			}
		}, gameEntities(), true);
	}

	void moveMonsters()
	{
		entitiesVisitor([&](Entity *e, PositionComponent &po, MovementComponent &mv, MonsterComponent &mo) {
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
				gameSpeed = 1;
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

			if (mo.affected(DamageTypeEnum::Water) && !mo.affected(DamageTypeEnum::Fire))
				mv.timeEnd += 3 * (mv.timeEnd - mv.timeStart) / 2;
			if (mo.affected(DamageTypeEnum::Fire) && !mo.affected(DamageTypeEnum::Water))
				mv.timeEnd -= 1 * (mv.timeEnd - mv.timeStart) / 2;
		}, gameEntities(), true);
	}

	void engineUpdate()
	{
		if (!globalGrid)
			return;
		updateMonsterAnimations();
	}

	void gameUpdate()
	{
		killMonsters();
		moveMonsters();
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

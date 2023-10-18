#include <bitset>

#include "../game.h"
#include "../grid.h"

#include <cage-core/entitiesVisitor.h>
#include <cage-core/profiling.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

namespace mazetd
{
	bool MonsterComponent::affected(DamageTypeEnum dmg) const
	{
		return dots[(uint32)dmg].damage > 0;
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
			const Real speed = gameReady ? gameSpeed : 0;
			entitiesVisitor(
				[&](Entity *e, const MovementComponent &mv, const MonsterComponent &mo, const EngineComponent &en)
				{
					Entity *f = en.entity;
					if (!f->has(aniComp))
						return;
					SkeletalAnimationComponent &a = f->value<SkeletalAnimationComponent>(aniComp);
					const uint32 moveDur = mv.timeEnd - mv.timeStart;
					const Real dist = stor(globalGrid->neighborDistance(mv.tileStart, mv.tileEnd));
					const Real moveSpeed = dist / (moveDur + 1);
					a.speed = 30 * speed * moveSpeed / mo.speed;
					CAGE_ASSERT(a.speed.valid() && a.speed.finite());
				},
				gameEntities(), false);
		}

		const auto engineUpdateListener = controlThread().update.listen(
			[]()
			{
				if (!globalGrid)
					return;
				updateMonsterAnimations();
			});

		const auto gameUpdateListener = eventGameUpdate().listen(
			[]()
			{
				ProfilingScope profiling("move monsters");

				entitiesVisitor(
					[&](Entity *e, PositionComponent &po, MovementComponent &mv, MonsterComponent &mo)
					{
						CAGE_ASSERT(po.tile == mv.tileEnd);
						if (gameTime < mv.timeEnd)
							return; // still moving

						for (uint32 i = 0; i < globalWaypoints->waypoints.size(); i++)
							if (globalWaypoints->waypoints[i]->tile == po.tile)
								mo.visitedWaypointsBits |= 1u << i;
						if (bitCount(mo.visitedWaypointsBits) == globalWaypoints->waypoints.size())
						{
							// the monster has reached its final waypoint
							playerHealth -= mo.damage;
							e->destroy();
							gameSpeed = min(gameSpeed, 1);
							return;
						}

						const auto go = globalWaypoints->find(po.tile, mo.visitedWaypointsBits);
						CAGE_ASSERT(go.tile != m);
						CAGE_ASSERT(go.distance > 0);
						mv.tileStart = po.tile;
						mv.tileEnd = po.tile = go.tile;
						mv.timeStart = gameTime;
						if (mo.timeToArrive > gameTime)
						{
							const uint64 distNext = globalGrid->neighborDistance(mv.tileStart, mv.tileEnd); // 24.8
							mv.timeEnd = numeric_cast<uint32>(gameTime + (mo.timeToArrive - gameTime) * distNext / go.distance);
						}
						mv.timeEnd = max(mv.timeEnd, gameTime + 1);

						if (mo.affected(DamageTypeEnum::Water) && !mo.affected(DamageTypeEnum::Fire))
							mv.timeEnd = gameTime + (mv.timeEnd - gameTime) * 2;
						if (mo.affected(DamageTypeEnum::Fire) && !mo.affected(DamageTypeEnum::Water))
							mv.timeEnd = gameTime + (mv.timeEnd - gameTime) / 2;
						mv.timeEnd = max(mv.timeEnd, gameTime + 1);
					},
					gameEntities(), true);
			});

		template<DamageTypeEnum A, DamageTypeEnum B>
		void dotsEliminateOposing(MonsterComponent &mo)
		{
			constexpr uint32 a = (uint32)A;
			constexpr uint32 b = (uint32)B;
			auto &dots = mo.dots;
			if (dots[a].duration > 0 && dots[b].duration > 0)
			{
				dots[a].damage = 0;
				dots[b].damage = 0;
				dots[a].duration = 30; // leave trail so that the elements are eliminated even short time later
				dots[b].duration = 30;
			}
		}

		template<DamageTypeEnum Type>
		void applyDot(MonsterComponent &mo, const Vec3 &mpp)
		{
			constexpr DamageTypeEnum strengthen = Type == DamageTypeEnum::Physical ? DamageTypeEnum::Poison : DamageTypeEnum::Magic;
			constexpr DamageTypeFlags dmgFlags = DamageTypeFlags(1u << (uint32)Type);
			const bool super = mo.affected(strengthen);
			const bool resist = any(dmgFlags & mo.resistances);

			auto &dot = mo.dots[(uint32)Type];
			uint32 dmg = dot.damage;
			if (dot.duration > 1)
				dmg /= dot.duration;
			if (dot.duration > 0)
				dot.duration--;
			if (dmg == 0)
				return;

			dot.damage -= dmg;
			if (super)
				dmg *= 2;
			if (resist)
				dmg /= 4;
			mo.life -= dmg;

			EffectConfig cfg;
			cfg.pos1 = mpp;
			const uint32 cnt = dot.duration > 0 ? 1 : 3;
			for (uint32 i = 0; i < cnt; i++)
			{
				const Vec3 d = randomDirection3();
				cfg.pos2 = cfg.pos1 + d * (super ? 0.5 : 0.3);
				cfg.type = Type;
				renderEffect(cfg);
			}
		}

		Vec3 monsterPosition(Entity *e)
		{
			Vec3 p = e->has<MovementComponent>() ? e->value<MovementComponent>().position() : e->value<PositionComponent>().position();
			return p + Vec3(0, e->value<PivotComponent>().elevation, 0);
		}

		const auto gameUpdateListener2 = eventGameUpdate().listen(
			[]()
			{
				ProfilingScope profiling("damage monsters");

				entitiesVisitor(
					[&](Entity *e, MonsterComponent &mo)
					{
						const Vec3 mpp = monsterPosition(e);
						dotsEliminateOposing<DamageTypeEnum::Fire, DamageTypeEnum::Water>(mo);
						dotsEliminateOposing<DamageTypeEnum::Poison, DamageTypeEnum::Magic>(mo);
						applyDot<DamageTypeEnum::Physical>(mo, mpp);
						applyDot<DamageTypeEnum::Fire>(mo, mpp);
						applyDot<DamageTypeEnum::Water>(mo, mpp);
						applyDot<DamageTypeEnum::Poison>(mo, mpp);
						applyDot<DamageTypeEnum::Magic>(mo, mpp);
						if (mo.life <= 0)
						{
							playerMoney += mo.money;
							createMonsterGhost(e);
							e->destroy();
						}
					},
					gameEntities(), true);
			},
			51); // right after attacks
	}
}

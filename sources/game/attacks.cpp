#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>

namespace
{
	struct Sorting
	{
		Entity *e = nullptr;
		vec3 p; // position
		real d; // distance to the tower
	};

	Sorting sorting(uint32 name, const vec3 &towerPosition)
	{
		Sorting m;
		m.e = gameEntities()->get(name);
		m.p = m.e->has<MovementComponent>() ? m.e->value<MovementComponent>().position() : globalGrid->center(m.e->value<PositionComponent>().tile);
		m.d = distance(m.p, towerPosition);
		return m;
	}

	struct Monster : public Sorting
	{
		MonsterComponent *m = nullptr;
	};

	Monster monster(uint32 name, const vec3 &towerPosition)
	{
		Monster m;
		(Sorting &)m = sorting(name, towerPosition);
		m.m = &m.e->value<MonsterComponent>();
		return m;
	}

	bool findTargetMonsters(const AttackComponent &att, SpatialQuery *monstersQuery, std::vector<Monster> &monsters, const vec3 &mp)
	{
		monsters.clear();
		monstersQuery->intersection(Sphere(mp, att.firingRange));
		if (monstersQuery->result().empty())
			return false;

		monsters.reserve(monstersQuery->result().size());
		for (uint32 n : monstersQuery->result())
			monsters.push_back(monster(n, mp));
		std::partial_sort(monsters.begin(), monsters.begin() + 1, monsters.end(), [](const Monster &a, const Monster &b) {
			return a.m->timeToArrive < b.m->timeToArrive;
			});
		return true;
	}

	void applyAugment(AttackComponent &att, SpatialQuery *buildingsQuery, const vec3 &mp, const vec3 &mpp)
	{
		if (!att.useAugments)
			return;
		buildingsQuery->intersection(Sphere(mp, 5));
		std::vector<Sorting> augs;
		augs.reserve(buildingsQuery->result().size());
		for (uint32 n : buildingsQuery->result())
		{
			Sorting s = sorting(n, mp);
			if (s.e->has<AugmentComponent>())
				augs.push_back(s);
		}
		if (!augs.empty())
		{
			std::partial_sort(augs.begin(), augs.begin() + 1, augs.end(), [](const Sorting &a, const Sorting &b) {
				return a.d < b.d;
				});

			const Sorting &aug = augs[0];
			ManaStorageComponent &mn = aug.e->value<ManaStorageComponent>();
			if (mn.mana >= att.damage)
			{
				mn.mana -= att.damage;

				{
					const AugmentComponent &a = aug.e->value<AugmentComponent>();
					att.damage *= 10;
					att.damageType = a.damageType;
					att.effectType = a.effectType;
				}

				{ // effect from the augment to the tower
					EffectConfig cfg;
					cfg.pos1 = aug.p + vec3(0, aug.e->value<PivotComponent>().elevation, 0);
					cfg.pos2 = mpp;
					cfg.type = att.effectType;
					renderEffect(cfg);
				}
			}
		}
	}

	void applySplash(const AttackComponent &att, SpatialQuery *monstersQuery, std::vector<Monster> &monsters, const vec3 &mp)
	{
		if (att.splashRadius > 0)
		{
			// search for all monsters inside the splash radius
			monstersQuery->intersection(Sphere(monsters[0].p, att.splashRadius));
			CAGE_ASSERT(!monstersQuery->result().empty());
			monsters.clear();
			monsters.reserve(monstersQuery->result().size());
			for (uint32 n : monstersQuery->result())
				monsters.push_back(monster(n, mp));
		}
		else
		{
			// single target - erase other monsters
			monsters.erase(monsters.begin() + 1, monsters.end());
		}
	}

	void applyAttack(const Monster &m, const AttackComponent &att)
	{
		if (att.damageType == DamageTypeEnum::None)
			return;

		m.m->life -= att.damage;
	}

	void applyDebuff(const Monster &m, const AttackComponent &att)
	{
		if (att.debuffType == DebuffTypeEnum::None)
			return;

		MonsterDebuffComponent &md = m.e->value<MonsterDebuffComponent>();
		md.type = att.debuffType;
		md.endTime = gameTime + 30 * 5;
	}

	void engineUpdate()
	{
		if (!gameRunning)
			return;
		SpatialQuery *monstersQuery = spatialMonsters();
		SpatialQuery *buildingsQuery = spatialStructures();
		std::vector<Monster> monsters;
		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, AttackComponent &attOrig) {
			if (attOrig.firingDelay > 0)
			{
				attOrig.firingDelay--;
				return;
			}

			const vec3 mp = globalGrid->center(pos.tile);
			const vec3 mpp = mp + vec3(0, e->value<PivotComponent>().elevation, 0);

			if (!findTargetMonsters(attOrig, monstersQuery, monsters, mp))
				return;

			AttackComponent att = attOrig;
			applyAugment(att, buildingsQuery, mp, mpp);

			{ // effect from the tower to the monster
				const Monster &m = monsters[0];
				EffectConfig cfg;
				cfg.pos1 = mpp;
				cfg.pos2 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
				cfg.type = att.effectType;
				renderEffect(cfg);
			}

			applySplash(att, monstersQuery, monsters, mp);

			for (const Monster &m : monsters)
			{
				applyAttack(m, att);
				applyDebuff(m, att);

				{ // effect from the monster
					EffectConfig cfg;
					cfg.pos1 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
					for (uint32 i = 0; i < 3; i++)
					{
						vec3 d = randomDirection3();
						d[1] = abs(d[1]);
						cfg.pos2 = cfg.pos1 + d * 0.5;
						cfg.type = att.effectType;
						renderEffect(cfg);
					}
				}
			}

			attOrig.firingDelay += att.firingPeriod;
		});
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, 50);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

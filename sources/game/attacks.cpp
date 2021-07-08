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

	void engineUpdate()
	{
		SpatialQuery *monstersQuery = spatialMonsters();
		SpatialQuery *buildingsQuery = spatialStructures();
		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, AttackComponent &attOrig) {
			if (attOrig.firingDelay > 0)
			{
				attOrig.firingDelay--;
				return;
			}

			const vec3 mp = globalGrid->center(pos.tile);
			const vec3 mpp = mp + vec3(0, e->value<PivotComponent>().elevation, 0);

			monstersQuery->intersection(Sphere(mp, attOrig.firingRange));
			if (monstersQuery->result().empty())
				return;

			AttackComponent att = attOrig;
			if (att.useAugments)
			{
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
					//ManaStorageComponent &mn = aug.e->value<ManaStorageComponent>();
					//if (mn.mana >= att.damage)
					{
						//mn.mana -= att.damage;

						{
							const AugmentComponent &a = aug.e->value<AugmentComponent>();
							att.damageType = a.damageType;
							att.damageDuration += a.damageDuration;
							att.damage *= 10;
						}

						{ // effect from the augment to the tower
							EffectConfig cfg;
							cfg.pos1 = aug.p + vec3(0, aug.e->value<PivotComponent>().elevation, 0);
							cfg.pos2 = mpp;
							cfg.type = att.damageType;
							renderEffect(cfg);
						}
					}
				}
			}

			std::vector<Monster> monsters;
			monsters.reserve(monstersQuery->result().size());
			for (uint32 n : monstersQuery->result())
				monsters.push_back(monster(n, mp));
			std::partial_sort(monsters.begin(), monsters.begin() + 1, monsters.end(), [](const Monster &a, const Monster &b) {
				return a.m->timeToArrive < b.m->timeToArrive;
			});

			{ // effect from the tower to the monster
				const Monster &m = monsters[0];
				EffectConfig cfg;
				cfg.pos1 = mpp;
				cfg.pos2 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
				cfg.type = att.damageType;
				renderEffect(cfg);
			}

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

			for (const Monster &m : monsters)
			{
				m.m->life -= att.damage;

				{ // effect from the monster
					EffectConfig cfg;
					cfg.pos1 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
					for (uint32 i = 0; i < 3; i++)
					{
						vec3 d = randomDirection3();
						d[1] = abs(d[1]);
						cfg.pos2 = cfg.pos1 + d * 0.5;
						cfg.type = att.damageType;
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

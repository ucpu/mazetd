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
	struct Monster
	{
		Entity *e = nullptr;
		MonsterComponent *m = nullptr;
		vec3 p; // position
		real d; // distance to the tower
	};

	Monster monster(uint32 name, const vec3 &towerPosition)
	{
		Monster m;
		m.e = gameEntities()->get(name);
		m.m = &m.e->value<MonsterComponent>();
		m.p = m.e->value<MovementComponent>().position();
		m.d = distance(m.p, towerPosition);
		return m;
	}

	void engineUpdate()
	{
		SpatialQuery *monstersQuery = spatialMonsters();
		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, AttackComponent &att) {
			if (att.firingDelay > 0)
			{
				att.firingDelay--;
				return;
			}

			monstersQuery->intersection(Sphere(globalGrid->center(pos.tile), att.firingRange));
			if (monstersQuery->result().empty())
				return;

			const vec3 mp = globalGrid->center(pos.tile);

			std::vector<Monster> monsters;
			monsters.reserve(monstersQuery->result().size());
			for (uint32 n : monstersQuery->result())
				monsters.push_back(monster(n, mp));

			std::partial_sort(monsters.begin(), monsters.begin() + 1, monsters.end(), [](const Monster &a, const Monster &b) {
				return a.m->timeToArrive < b.m->timeToArrive;
			});

			{ // effect from tower to the monster
				const Monster &m = monsters[0];
				EffectConfig cfg;
				cfg.pos1 = mp + vec3(0, e->value<PivotComponent>().elevation, 0);
				cfg.pos2 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
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
						renderEffect(cfg);
					}
				}
			}

			att.firingDelay += att.firingPeriod;
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

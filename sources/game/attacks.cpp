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

			struct Monster
			{
				Entity *e = nullptr;
				MonsterComponent *m = nullptr;
				vec3 p; // position
				real d; // distance to the tower
			};

			std::vector<Monster> monsters;
			monsters.reserve(monstersQuery->result().size());
			for (uint32 n : monstersQuery->result())
			{
				Monster m;
				m.e = gameEntities()->get(n);
				m.m = &m.e->value<MonsterComponent>();
				m.p = m.e->value<MovementComponent>().position();
				m.d = distance(m.p, mp);
				monsters.push_back(m);
			}

			std::partial_sort(monsters.begin(), monsters.begin() + 1, monsters.end(), [](const Monster &a, const Monster &b) {
				return a.m->timeToArrive < b.m->timeToArrive;
			});

			const Monster &m = monsters[0];
			m.m->life -= att.damage;

			// todo splash

			{
				EffectConfig cfg;
				cfg.pos1 = mp + vec3(0, e->value<PivotComponent>().elevation, 0);
				cfg.pos2 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
				renderEffect(cfg);
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

#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <algorithm>
#include <vector>

namespace
{
	struct PotentialComponent
	{
		real potential;
		real modified;
	};

	void engineInit()
	{
		gameEntities()->defineComponent(PotentialComponent());
	}

	struct Sorting
	{
		Entity *e = nullptr;
		ManaStorageComponent *s = nullptr;
		uint32 transfer = 0;
	};

	void engineUpdate()
	{
		if (!gameRunning)
			return;

		EntityComponent *recvComp = gameEntities()->component<ManaReceiverComponent>();
		EntityComponent *distrComp = gameEntities()->component<ManaDistributorComponent>();
		EntityComponent *storComp = gameEntities()->component<ManaStorageComponent>();
		EntityComponent *potenComp = gameEntities()->component<PotentialComponent>();
		SpatialQuery *buildingsQuery = spatialStructures();

		if ((gameTime % 5) == 0)
		{
			// distribute mana
			entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &distr, const PotentialComponent &pot, ManaStorageComponent &stor) {
				if (stor.mana == 0)
					return;

				const vec3 mp = globalGrid->center(pos.tile);
				buildingsQuery->intersection(Sphere(mp, distr.range));
				std::vector<Sorting> recvs;
				recvs.reserve(buildingsQuery->result().size());
				for (uint32 n : buildingsQuery->result())
				{
					if (n == e->name())
						continue;
					Sorting s;
					s.e = gameEntities()->get(n);
					if (!s.e->has(recvComp))
						continue;
					if (s.e->value<PotentialComponent>(potenComp).potential >= pot.potential)
						continue;
					s.s = &s.e->value<ManaStorageComponent>(storComp);
					s.transfer = min(s.s->capacity >= s.s->mana ? s.s->capacity - s.s->mana : 0u, min(distr.transferLimit, stor.mana));
					if (s.transfer == 0)
						continue;
					recvs.push_back(s);
				}

				if (recvs.empty())
					return;

				std::partial_sort(recvs.begin(), recvs.begin() + 1, recvs.end(), [](const Sorting &a, const Sorting &b) {
					return a.transfer > b.transfer;
				});

				const Sorting &r = recvs[0];
				CAGE_ASSERT(stor.mana >= r.transfer);
				stor.mana -= r.transfer;
				r.s->mana += r.transfer;
				CAGE_ASSERT(r.s->mana <= r.s->capacity);

				{
					EffectConfig cfg;
					cfg.pos1 = mp + vec3(0, e->value<PivotComponent>().elevation, 0);
					cfg.pos2 = globalGrid->center(r.e->value<PositionComponent>().tile) + vec3(0, r.e->value<PivotComponent>().elevation, 0);
					cfg.type = EffectTypeEnum::Mana;
					renderEffect(cfg);
				}
			});

			// reset potentials
			entitiesVisitor(gameEntities(), [&](Entity *e, const ManaStorageComponent &stor) {
				PotentialComponent &pot = e->value<PotentialComponent>(potenComp);
				const bool r = e->has(recvComp);
				const bool d = e->has(distrComp);
				CAGE_ASSERT(r || d);
				if (!d)
					pot.modified = 0;
				else if (!r)
					pot.modified = 1;
				else
					pot.modified = real(stor.mana) / stor.capacity;
				pot.potential = pot.modified;
			});
		}
		else for (uint32 iter = 0; iter < 3; iter++)
		{
			// spread potentials
			entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &distr, PotentialComponent &pot) {
				uint32 cnt = 5;
				real sum = pot.potential * cnt;
				buildingsQuery->intersection(Sphere(globalGrid->center(pos.tile), distr.range));
				for (uint32 n : buildingsQuery->result())
				{
					if (n == e->name())
						continue;
					Entity *r = gameEntities()->get(n);
					if (!r->has(recvComp))
						continue;
					const real pt = r->value<PotentialComponent>(potenComp).potential;
					if (pt > pot.potential && !r->has(distrComp))
						continue;
					sum += pt;
					cnt++;
				}
				pot.modified = sum / cnt;
			});

			// apply modified
			entitiesVisitor(gameEntities(), [&](PotentialComponent &pot) {
				pot.potential = pot.modified;
			});
		}
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update, 35); // after spatial update
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

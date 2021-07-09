#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <random>
#include <algorithm>
#include <vector>

namespace
{
	struct Sorting
	{
		Entity *e = nullptr;
		ManaStorageComponent *s = nullptr;
		uint32 transfer = 0;
		real ratio = real::Nan();
		bool hasDistributor = false;
	};

	void engineUpdate()
	{
		if (!gameRunning)
			return;

		std::mt19937 rng({});

		EntityComponent *recvComp = gameEntities()->component<ManaReceiverComponent>();
		EntityComponent *distrComp = gameEntities()->component<ManaDistributorComponent>();
		EntityComponent *storComp = gameEntities()->component<ManaStorageComponent>();
		SpatialQuery *buildingsQuery = spatialStructures();

		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &distr, ManaStorageComponent &stor) {
			if (stor.mana == 0)
				return;
			if (((e->name() + gameTime) % distr.period) != 0)
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
				s.s = &s.e->value<ManaStorageComponent>(storComp);
				s.transfer = min(s.s->capacity >= s.s->mana ? s.s->capacity - s.s->mana : 0u, min(distr.transferLimit, stor.mana));
				if (s.transfer == 0)
					continue;
				s.ratio = real(s.s->mana + s.transfer) / s.s->capacity;
				s.hasDistributor = s.e->has(distrComp);
				recvs.push_back(s);
			}

			/*
			if (e->has(recvComp))
			{
				// we are a relay -> careful balancing is required
				recvs.erase(std::remove_if(recvs.begin(), recvs.end(), [&](const Sorting &a) {
					return a.hasDistributor && a.ratio > real(stor.mana + 1e-5) / stor.capacity;
				}), recvs.end());
			}
			*/

			if (recvs.empty())
				return;

			std::shuffle(recvs.begin(), recvs.end(), rng);
			std::partial_sort(recvs.begin(), recvs.begin() + 1, recvs.end(), [](const Sorting &a, const Sorting &b) {
				return a.ratio < b.ratio;
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
				cfg.type = DamageTypeEnum::Mana;
				renderEffect(cfg);
			}
		});
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, 35); // after spatial update
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-core/profiling.h>
#include <cage-engine/scene.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>

namespace
{
	struct EffectorsUpdater
	{
		struct Neighbor
		{
			Entity *e = nullptr;
			Real dist2;
		};

		SpatialQuery *buildingsQuery = spatialStructures();
		EntityComponent *compPosition = gameEntities()->component<PositionComponent>();
		EntityComponent *compModElement = gameEntities()->component<ModElementComponent>();
		EntityComponent *compModBonus = gameEntities()->component<ModBonusComponent>();
		EntityComponent *compModTargeting = gameEntities()->component<ModTargetingComponent>();
		std::vector<Neighbor> neighbors;

		void findNeighbors(Entity *me)
		{
			neighbors.clear();
			const uint32 myName = me->name();
			const Vec3 myPos = globalGrid->center(me->value<PositionComponent>(compPosition).tile);
			constexpr Real Range = 5;
			buildingsQuery->intersection(Sphere(myPos, Range + 2)); // extra threshold because the spatial search happens in 3D but the resulting distance is measured in 2D
			for (uint32 nn : buildingsQuery->result())
			{
				if (nn == myName)
					continue;
				Neighbor n;
				n.e = gameEntities()->get(nn);
				n.dist2 = distanceSquared(globalGrid->center(n.e->value<PositionComponent>(compPosition).tile), myPos);
				if (n.dist2 < sqr(Range))
					neighbors.push_back(n);
			}
			std::sort(neighbors.begin(), neighbors.end(), [](const Neighbor &a, const Neighbor &b) {
				return a.dist2 < b.dist2;
			});
		}

		const Neighbor *closestNeighbor(EntityComponent *comp)
		{
			for (const auto &it : neighbors)
			{
				if (it.e->has(comp))
					return &it;
			}
			return nullptr;
		}

		void findMods(AttackComponent &a)
		{
			if (const Neighbor *n = closestNeighbor(compModElement))
			{
				a.element = n->e->value<ModElementComponent>(compModElement).element;
				a.effectors[0] = n->e;
			}
			if (const Neighbor *n = closestNeighbor(compModBonus))
			{
				a.bonus = n->e->value<ModBonusComponent>(compModBonus).bonus;
				a.effectors[1] = n->e;
			}
			if (const Neighbor *n = closestNeighbor(compModTargeting))
			{
				a.targeting = n->e->value<ModTargetingComponent>(compModTargeting).targeting;
				a.effectors[2] = n->e;
			}
		}

		void run()
		{
			entitiesVisitor([&](AttackComponent &a) {
				const auto firingDelay = a.firingDelay;
				a = {};
				a.firingDelay = firingDelay;
			}, gameEntities(), false);

			entitiesVisitor([&](Entity *me, AttackComponent &a) {
				findNeighbors(me);
				findMods(a);
			}, gameEntities(), false);
		}
	};
}

void updateAttacksMods()
{
	ProfilingScope profiling("update attacks mods", "player");
	EffectorsUpdater updater;
	updater.run();
}

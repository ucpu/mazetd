#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-core/profiling.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>

namespace
{
	struct Edge
	{
		sint32 limit = 0, flow = 0;
		sint32 avail() const { return limit - flow; }
	};

	struct Node
	{
		std::map<Entity *, Edge> edges;
		ManaStorageComponent *stor = nullptr;
		Entity *prev = nullptr;
		bool visited = false;

		std::vector<const decltype(edges)::value_type *> sorted() const
		{
			std::vector<const decltype(edges)::value_type *> v;
			for (const auto &it : edges)
				v.push_back(&it);
			std::sort(v.begin(), v.end(), [](const auto &a, const auto &b) {
				ManaStorageComponent *aa = nodes[a->first].stor;
				ManaStorageComponent *bb = nodes[b->first].stor;
				if (aa && bb)
					return aa->mana < bb->mana; // prefer filling buildings with less mana
				return bb < aa;
			});
			return v;
		}
	};

	static Entity *const superSource = (Entity *)1;
	static Entity *const superSink = (Entity *)2;
	std::unordered_map<Entity *, Node> nodes;
	std::vector<Entity *> queue;

	bool bfs()
	{
		for (auto &it : nodes)
		{
			it.second.prev = nullptr;
			it.second.visited = false;
		}
		queue.clear();
		queue.push_back(superSource);
		nodes[superSource].visited = true;
		while (!queue.empty())
		{
			Entity *u = queue.front();
			queue.erase(queue.begin());
			for (const auto &it : nodes[u].sorted())
			{
				const auto &[v, edg] = *it;
				if (nodes[v].visited || edg.flow >= edg.limit)
					continue;
				queue.push_back(v);
				nodes[v].visited = true;
				nodes[v].prev = u;
			}
		}
		return nodes[superSink].visited;
	}

	// ford-fulkerson algorithm
	void findMaximumFlow()
	{
		while (bfs())
		{
			sint32 f = m;
			Entity *s = superSink;
			while (s != superSource)
			{
				const sint32 av = nodes[nodes[s].prev].edges[s].avail();
				CAGE_ASSERT(av >= 0);
				f = min(f, av);
				s = nodes[s].prev;
			}
			CAGE_ASSERT(f >= 0 && f < 1000000);
			if (f == 0)
				break;
			s = superSink;
			while (s != superSource)
			{
				Entity *p = nodes[s].prev;
				nodes[p].edges[s].flow += f;
				nodes[s].edges[p].flow -= f;
				s = p;
			}
		}
	}

	void applyTransfer()
	{
		for (auto &it : nodes)
		{
			Entity *u = it.first;
			for (auto &[v, e] : nodes[u].edges)
			{
				if (e.flow <= 0)
					continue;
				ManaStorageComponent *a = nodes[u].stor;
				ManaStorageComponent *b = nodes[v].stor;
				if (!a || !b)
					continue;
				CAGE_ASSERT(e.flow <= e.limit);
				a->mana -= e.flow;
				b->mana += e.flow;
				EffectConfig cfg;
				cfg.pos1 = u->value<PositionComponent>().position() + Vec3(0, u->value<PivotComponent>().elevation, 0);
				cfg.pos2 = v->value<PositionComponent>().position() + Vec3(0, v->value<PivotComponent>().elevation, 0);
				cfg.type = DamageTypeEnum::Magic;
				renderEffect(cfg);
			}
		}
	}

	void gameUpdate()
	{
		if ((gameTime % 6) != 0)
			return;

		ProfilingScope profiling("mana transfers");

		SpatialQuery *buildingsQuery = spatialStructures();
		nodes.clear();

		// generate the graph
		entitiesVisitor([&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &distr, ManaStorageComponent &stor) {
			buildingsQuery->intersection(Sphere(globalGrid->center(pos.tile) * Vec3(1, 0, 1), distr.range));
			for (uint32 n : buildingsQuery->result())
			{
				if (n == e->name())
					continue;
				Entity *r = gameEntities()->get(n);
				if (!r->has<ManaReceiverComponent>())
					continue;
				nodes[e].edges[r].limit = distr.transferLimit;
				nodes[e].stor = &stor;
			}
			if (!e->has<ManaReceiverComponent>()) // this node is source
				nodes[superSource].edges[e].limit = stor.mana; // allow all mana to be transferred away
		}, gameEntities(), false);

		// find all sinks
		Real minStor = Real::Infinity();
		entitiesVisitor([&](Entity *e, const PositionComponent &pos, const ManaReceiverComponent &, ManaStorageComponent &stor) {
			if (!e->has<ManaDistributorComponent>()) // this node is sink
			{
				nodes[e].edges[superSink].limit = stor.capacity - stor.mana; // allow to fill up the whole storage
				nodes[e].stor = &stor;
				if (stor.capacity > 0)
					minStor = min(minStor, Real(stor.mana) / stor.capacity);
			}
		}, gameEntities(), false);

		// storage nodes
		entitiesVisitor([&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &, const ManaReceiverComponent &, ManaStorageComponent &stor) {
			if (stor.capacity == 0)
				return;
			if (Real(stor.mana) / stor.capacity < minStor)
			{ // treat as sink
				nodes[e].edges[superSink].limit = stor.capacity - stor.mana; // allow to fill up the whole storage
				nodes[e].stor = &stor;
			}
			else
			{ // treat as source
				nodes[superSource].edges[e].limit = stor.mana; // allow all mana to be transferred away
			}
		}, gameEntities(), false);

		// find maximum flow
		findMaximumFlow();

		// apply all transfers and generate effects
		applyTransfer();

#ifdef CAGE_DEBUG
		entitiesVisitor([&](Entity *e, ManaStorageComponent &stor) {
			CAGE_ASSERT(stor.mana <= stor.capacity);
		}, gameEntities(), false);
#endif // CAGE_DEBUG
	}

	struct Callbacks
	{
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameUpdateListener.attach(eventGameUpdate(), 35); // after spatial update
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

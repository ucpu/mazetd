#include <cage-core/entitiesVisitor.h>
#include <cage-core/profiling.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	struct Node
	{
		Vec3 pos3;
		Vec2 pos2;
		ManaStorageComponent *stor = nullptr;
		uint32 amount = 0;

		bool operator < (const Node &other) const
		{
			if (stor->transferOrdering == other.stor->transferOrdering)
				return amount > other.amount;
			return stor->transferOrdering < other.stor->transferOrdering;
		}
	};

	std::vector<Node> distributors, consumers;

	std::pair<Real, uint32> findBest(const Node &a, PointerRange<const Node> bs)
	{
		Real bc = 0;
		uint32 bi = m;
		for (const Node &b : bs)
		{
			if (a.stor == b.stor)
				continue;
			const uint32 am = min(a.amount, b.amount);
			if (am < 50)
				continue;
			const Real d = distance(a.pos2, b.pos2) + 1;
			const Real c = am / d;
			if (c > bc)
			{
				bc = c;
				bi = &b - bs.begin();
			}
		}
		return { bc, bi };
	}

	void transfer(uint32 di, uint32 ci)
	{
		Node &d = distributors[di];
		Node &c = consumers[ci];
		CAGE_ASSERT(d.stor != c.stor);
		const uint32 am = min(d.amount, c.amount);
		CAGE_ASSERT(d.stor->mana >= am);
		CAGE_ASSERT(c.stor->mana + am <= c.stor->capacity);
		d.amount -= am;
		c.amount -= am;
		d.stor->mana -= am;
		c.stor->mana += am;
		EffectConfig cfg;
		cfg.pos1 = d.pos3;
		cfg.pos2 = c.pos3;
		cfg.type = DamageTypeEnum::Mana;
		renderEffect(cfg);
		if (d.amount == 0)
			distributors.erase(distributors.begin() + di);
		if (c.amount == 0)
			consumers.erase(consumers.begin() + ci);
	}

	void gameUpdate()
	{
		ProfilingScope profiling("mana transfers");

		distributors.clear();
		distributors.reserve(100);
		consumers.clear();
		consumers.reserve(100);

		entitiesVisitor([&](Entity *e, const PositionComponent &pos, const ManaDistributorComponent &, ManaStorageComponent &stor) {
			Node n;
			n.amount = stor.mana;
			if (n.amount == 0)
				return;
			const Vec3 p = pos.position();
			n.pos3 = p + Vec3(0, e->value<PivotComponent>().elevation, 0);
			n.pos2 = Vec2(p[0], p[2]);
			n.stor = &stor;
			distributors.push_back(n);
		}, gameEntities(), false);

		entitiesVisitor([&](Entity *e, const PositionComponent &pos, const ManaReceiverComponent &, ManaStorageComponent &stor) {
			Node n;
			n.amount = stor.capacity - stor.mana;
			if (n.amount == 0)
				return;
			const Vec3 p = pos.position();
			n.pos3 = p + Vec3(0, e->value<PivotComponent>().elevation, 0);
			n.pos2 = Vec2(p[0], p[2]);
			n.stor = &stor;
			consumers.push_back(n);
		}, gameEntities(), false);

		while (!distributors.empty() && !consumers.empty())
		{
			std::sort(distributors.begin(), distributors.end());
			std::sort(consumers.begin(), consumers.end());
			const auto a = findBest(distributors[0], consumers);
			const auto b = findBest(consumers[0], distributors);
			if (a.second == m || b.second == m)
				break;
			if (a.first < b.first)
				transfer(0, a.second);
			else
				transfer(b.second, 0);
		}

#ifdef CAGE_ASSERT_ENABLED
		entitiesVisitor([&](Entity *e, ManaStorageComponent &stor) {
			CAGE_ASSERT(stor.mana <= stor.capacity);
		}, gameEntities(), false);
#endif // CAGE_ASSERT_ENABLED
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

#include <cage-core/enumerate.h>
#include <cage-core/pointerRangeHolder.h>

#include "generate.h"

#include <initializer_list>

namespace
{
	sint16 maxSlope(const Grid *g, uint32 idx)
	{
		const sint16 me = g->elevations[idx];
		const Vec2i mp = g->position(idx);
		sint16 r = 0;
		for (const Vec2i off : { Vec2i(-1, 0), Vec2i(1, 0), Vec2i(0, -1), Vec2i(0, 1) })
		{
			const Vec2i p = mp + off;
			const uint32 i = g->index(p);
			if (i == m)
				continue;
			const sint16 e = g->elevations[i];
			r = max(r, cage::abs(sint16(me - e)));
		}
		return r;
	}

	void fillUnreachable(Grid *g)
	{
		std::vector<uint16> comps;
		std::vector<uint32> stack;
		stack.reserve(1000);
		const uint32 total = numeric_cast<uint32>(g->flags.size());
		comps.resize(total, m);
		uint32 largestId = m;
		uint32 largestCnt = 0;
		uint32 c = 0;
		for (uint32 s = 0; s < total; s++)
		{
			if (g->flags[s] == TileFlags::Invalid)
				continue;
			if (comps[s] != m)
				continue;
			comps[s] = c;
			uint32 cnt = 1;
			CAGE_ASSERT(stack.empty());
			stack.push_back(s);
			while (!stack.empty())
			{
				const uint32 i = stack.back();
				stack.pop_back();
				const Vec2i mp = g->position(i);
				for (const Vec2i off : { Vec2i(-1, 0), Vec2i(1, 0), Vec2i(0, -1), Vec2i(0, 1) })
				{
					const uint32 j = g->index(mp + off);
					if (j == m)
						continue;
					if (g->flags[j] == TileFlags::Invalid)
						continue;
					if (comps[j] != m)
						continue;
					comps[j] = c;
					cnt++;
					stack.push_back(j);
				}
			}
			if (cnt > largestCnt)
			{
				largestCnt = cnt;
				largestId = c;
			}
			c++;
		}
		for (uint32 s = 0; s < total; s++)
			if (comps[s] != largestId)
				g->flags[s] = TileFlags::Invalid;
	}

	void findBorder(Grid *g)
	{
		const uint32 w = g->resolution[0];
		for (auto it : enumerate(g->flags))
		{
			const Vec2i mp = g->position(it.index);
			if (none(g->flags[it.index] & TileFlags::Invalid))
				continue;
			for (const Vec2i off : { Vec2i(-1, 0), Vec2i(1, 0), Vec2i(0, -1), Vec2i(0, 1) })
			{
				const uint32 j = g->index(mp + off);
				if (j == m)
					continue;
				if (none(g->flags[j] & TileFlags::Invalid))
					*it |= TileFlags::Border;
			}
		}
	}
}

uint32 Grid::index(Vec2i pos) const
{
	const Vec2i a = pos + gridOffset;
	if (a[0] < 0 || a[0] >= resolution[0] || a[1] < 0 || a[1] >= resolution[1])
		return m;
	return a[1] * resolution[0] + a[0];
}

uint32 Grid::index(Vec2 pos) const
{
	return index(Vec2i(pos + 200.5) - 200); 
}

uint32 Grid::index(Vec3 pos) const
{
	return index(Vec2(pos[0], pos[2]));
}

Vec2i Grid::position(uint32 idx) const
{
	const uint32 y = idx / resolution[0];
	const uint32 x = idx % resolution[0];
	return Vec2i(x, y) - gridOffset;
}

Vec3 Grid::center(uint32 idx) const
{
	const Vec2i p = position(idx);
	return Vec3(p[0], stor(elevations[idx]), p[1]);
}

uint32 Grid::neighborDistance(uint32 a, uint32 b) const
{
	// todo optimized integer only solution
	return rtos32(distance(center(a), center(b)));
}

Holder<Grid> newGrid(Holder<Procedural> procedural)
{
	Holder<Grid> g = systemMemory().createHolder<Grid>();
	g->resolution = Vec2i(141);
	g->gridOffset = Vec2i(70);
	const uint32 total = g->resolution[0] * g->resolution[1];
	{
		PointerRangeHolder<const sint16> vec;
		vec.resize(total);
		for (uint32 i = 0; i < total; i++)
			vec[i] = rtos16(clamp(procedural->elevation(Vec2(g->position(i))), -100, 100));
		g->elevations = vec;
	}
	{
		PointerRangeHolder<TileFlags> vec;
		vec.resize(total);
		constexpr sint16 waterThreshold = rtos16(-7);
		constexpr sint16 sunThreshold = rtos16(1);
		constexpr sint16 windThreshold = rtos16(6);
		constexpr sint16 slopeThreshold = rtos16(0.35);
		for (uint32 i = 0; i < total; i++)
		{
			if (g->elevations[i] < waterThreshold)
				vec[i] |= TileFlags::Water;
			else if (g->elevations[i] < sunThreshold)
				vec[i] |= TileFlags::Sun;
			else if (g->elevations[i] < windThreshold)
				vec[i] |= TileFlags::Wind;
			else
				vec[i] |= TileFlags::Snow;
			if (maxSlope(+g, i) > slopeThreshold)
				vec[i] = TileFlags::Invalid;
			if (lengthSquared(Vec2(g->position(i))) > sqr(55))
				vec[i] = TileFlags::Invalid;
		}
		g->flags = vec;
	}
	fillUnreachable(+g);
	findBorder(+g);
	{
		uint32 valid = 0;
		for (TileFlags f : g->flags)
			valid += none(f & TileFlags::Invalid);
		CAGE_LOG(SeverityEnum::Info, "mapgen", Stringizer() + "valid tiles: " + valid);
	}
	return g;
}

#ifdef CAGE_DEBUG
namespace
{
	struct GridTester
	{
		Holder<PointerRange<sint16>> makeElevations()
		{
			PointerRangeHolder<sint16> vec;
			vec.resize(10 * 5);
			return vec;
		}

		GridTester()
		{
			Grid grid;
			uint16 elevations[10 * 5] = {};
			grid.elevations = makeElevations();
			grid.gridOffset = Vec2i(4, 2);
			grid.resolution = Vec2i(10, 5);

			CAGE_ASSERT(grid.index(Vec2i()) == 24);
			CAGE_ASSERT(grid.index(Vec2i(-4, -2)) == 0);
			CAGE_ASSERT(grid.index(Vec2i(5, -2)) == 9);
			CAGE_ASSERT(grid.index(Vec2i(-4, 2)) == 40);
			CAGE_ASSERT(grid.index(Vec2i(5, 2)) == 49);
			CAGE_ASSERT(grid.index(Vec2i(-10, 0)) == m);
			CAGE_ASSERT(grid.index(Vec2i(0, 100)) == m);
			CAGE_ASSERT(grid.index(Vec2()) == 24);

			CAGE_ASSERT(grid.index(Vec2(-0.45, 0)) == 24);
			CAGE_ASSERT(grid.index(Vec2(+0.45, 0)) == 24);
			CAGE_ASSERT(grid.index(Vec2(0, -0.45)) == 24);
			CAGE_ASSERT(grid.index(Vec2(0, +0.45)) == 24);
			CAGE_ASSERT(grid.index(Vec2(4.9, 1.9)) == 49);
			CAGE_ASSERT(grid.index(Vec2(5.1, 1.9)) == 49);
			CAGE_ASSERT(grid.index(Vec2(4.9, 2.1)) == 49);
			CAGE_ASSERT(grid.index(Vec2(5.1, 2.1)) == 49);

			CAGE_ASSERT(grid.position(24) == Vec2i());
			CAGE_ASSERT(grid.position(0) == Vec2i(-4, -2));
			CAGE_ASSERT(grid.position(9) == Vec2i(5, -2));
			CAGE_ASSERT(grid.position(40) == Vec2i(-4, 2));
			CAGE_ASSERT(grid.position(49) == Vec2i(5, 2));

			CAGE_ASSERT(grid.center(0) == Vec3(-4, 0, -2));
			CAGE_ASSERT(grid.center(24) == Vec3());
			CAGE_ASSERT(grid.center(49) == Vec3(5, 0, 2));

			CAGE_ASSERT(grid.index(grid.position(13)) == 13);
			CAGE_ASSERT(grid.index(grid.position(42)) == 42);
			CAGE_ASSERT(grid.index(grid.center(13)) == 13);
			CAGE_ASSERT(grid.index(grid.center(42)) == 42);
		}
	} gridTester;
}
#endif // CAGE_DEBUG

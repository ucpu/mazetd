#include <cage-core/pointerRangeHolder.h>

#include "generate.h"

#include <initializer_list>

namespace
{
	sint16 maxSlope(const Grid *g, uint32 idx)
	{
		const sint16 me = g->elevations[idx];
		const ivec2 mp = g->position(idx);
		sint16 r = 0;
		for (const ivec2 off : { ivec2(-1, 0), ivec2(1, 0), ivec2(0, -1), ivec2(0, 1) })
		{
			const ivec2 p = mp + off;
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
				const ivec2 mp = g->position(i);
				for (const ivec2 off : { ivec2(-1, 0), ivec2(1, 0), ivec2(0, -1), ivec2(0, 1) })
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
}

uint32 Grid::index(ivec2 pos) const
{
	const ivec2 a = pos + gridOffset;
	if (a[0] < 0 || a[0] >= resolution[0] || a[1] < 0 || a[1] >= resolution[1])
		return m;
	return a[1] * resolution[0] + a[0];
}

uint32 Grid::index(vec2 pos) const
{
	return index(ivec2(pos + 200.5) - 200); // 200 is used for rounding towards negative infinity instead of zero
}

uint32 Grid::index(vec3 pos) const
{
	return index(vec2(pos[0], pos[2]));
}

ivec2 Grid::position(uint32 idx) const
{
	const uint32 y = idx / resolution[0];
	const uint32 x = idx % resolution[0];
	return ivec2(x, y) - gridOffset;
}

vec3 Grid::center(uint32 idx) const
{
	const ivec2 p = position(idx);
	return vec3(p[0], stor(elevations[idx]), p[1]);
}

uint32 Grid::neighborDistance(uint32 a, uint32 b) const
{
	// todo optimized integer only solution
	return rtos32(distance(center(a), center(b)));
}

Holder<Grid> newGrid(Holder<Procedural> procedural)
{
	Holder<Grid> g = systemMemory().createHolder<Grid>();
	g->resolution = ivec2(201);
	g->gridOffset = ivec2(100);
	const uint32 total = g->resolution[0] * g->resolution[1];
	{
		PointerRangeHolder<const sint16> vec;
		vec.resize(total);
		for (uint32 i = 0; i < total; i++)
			vec[i] = rtos16(clamp(procedural->elevation(vec2(g->position(i))), -100, 100));
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
		}
		g->flags = vec;
	}
	fillUnreachable(+g);
	{
		uint32 valid = 0;
		for (TileFlags f : g->flags)
			valid += none(f & TileFlags::Invalid);
		CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "valid tiles: " + valid);
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
			grid.gridOffset = ivec2(4, 2);
			grid.resolution = ivec2(10, 5);

			CAGE_ASSERT(grid.index(ivec2()) == 24);
			CAGE_ASSERT(grid.index(ivec2(-4, -2)) == 0);
			CAGE_ASSERT(grid.index(ivec2(5, -2)) == 9);
			CAGE_ASSERT(grid.index(ivec2(-4, 2)) == 40);
			CAGE_ASSERT(grid.index(ivec2(5, 2)) == 49);
			CAGE_ASSERT(grid.index(ivec2(-10, 0)) == m);
			CAGE_ASSERT(grid.index(ivec2(0, 100)) == m);
			CAGE_ASSERT(grid.index(vec2()) == 24);

			CAGE_ASSERT(grid.index(vec2(-0.45, 0)) == 24);
			CAGE_ASSERT(grid.index(vec2(+0.45, 0)) == 24);
			CAGE_ASSERT(grid.index(vec2(0, -0.45)) == 24);
			CAGE_ASSERT(grid.index(vec2(0, +0.45)) == 24);
			CAGE_ASSERT(grid.index(vec2(4.9, 1.9)) == 49);
			CAGE_ASSERT(grid.index(vec2(5.1, 1.9)) == 49);
			CAGE_ASSERT(grid.index(vec2(4.9, 2.1)) == 49);
			CAGE_ASSERT(grid.index(vec2(5.1, 2.1)) == 49);

			CAGE_ASSERT(grid.position(24) == ivec2());
			CAGE_ASSERT(grid.position(0) == ivec2(-4, -2));
			CAGE_ASSERT(grid.position(9) == ivec2(5, -2));
			CAGE_ASSERT(grid.position(40) == ivec2(-4, 2));
			CAGE_ASSERT(grid.position(49) == ivec2(5, 2));

			CAGE_ASSERT(grid.center(0) == vec3(-4, 0, -2));
			CAGE_ASSERT(grid.center(24) == vec3());
			CAGE_ASSERT(grid.center(49) == vec3(5, 0, 2));

			CAGE_ASSERT(grid.index(grid.position(13)) == 13);
			CAGE_ASSERT(grid.index(grid.position(42)) == 42);
			CAGE_ASSERT(grid.index(grid.center(13)) == 13);
			CAGE_ASSERT(grid.index(grid.center(42)) == 42);
		}
	} gridTester;
}
#endif // CAGE_DEBUG

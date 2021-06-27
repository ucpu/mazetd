#include <cage-core/pointerRangeHolder.h>

#include "generate.h"

namespace
{
	CAGE_FORCE_INLINE sint16 rtos16(real v)
	{
		return numeric_cast<sint16>(v * 255);
	}

	CAGE_FORCE_INLINE real s16tor(sint16 v)
	{
		return v / 255.f;
	}

	CAGE_FORCE_INLINE sint32 rtos32(real v)
	{
		return numeric_cast<sint32>(v * 255 * 255);
	}

	CAGE_FORCE_INLINE real s32tor(sint32 v)
	{
		return v / 255.f / 255.f;
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
	return vec3(p[0], s16tor(elevations[idx]), p[1]);
}

uint32 Grid::neighborDistance(uint32 a, uint32 b) const
{
	return 0; // todo
}

Holder<Grid> newGrid(Holder<Procedural> procedural)
{
	Holder<Grid> g = systemMemory().createHolder<Grid>();
	g->resolution = ivec2(201);
	g->gridOffset = ivec2(100);
	{
		PointerRangeHolder<const sint16> vec;
		vec.resize(g->resolution[0] * g->resolution[1]);
		g->elevations = vec;
	}
	{
		PointerRangeHolder<TileFlags> vec;
		vec.resize(g->resolution[0] * g->resolution[1]);
		g->tiles = vec;
	}
	return g;
}

Holder<Grid> globalGrid;

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

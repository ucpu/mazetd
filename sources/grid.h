#include <cage-core/math.h>

using namespace cage;

CAGE_FORCE_INLINE constexpr sint16 rtos16(real v)
{
	return numeric_cast<sint16>(v * 255);
}

CAGE_FORCE_INLINE constexpr sint32 rtos32(real v)
{
	return numeric_cast<sint32>(v * 255);
}

CAGE_FORCE_INLINE constexpr real stor(sint32 v)
{
	constexpr real n = 1.0 / 255;
	return v * n;
}

enum class TileFlags : uint8
{
	None = 0,
	Invalid = 1u << 0, // tile is outside playable area
	Water = 1u << 1,
	Wall = 1u << 5, // walls, towers and other impassable structures built by the player
	Trap = 1u << 6, // passable structures built by the player
	Spawn = 1u << 7, // passable tile that player may not build on
};

namespace cage
{
	GCHL_ENUM_BITS(TileFlags);
}

struct Grid : private Immovable
{
	ivec2 gridOffset;
	ivec2 resolution;

	Holder<PointerRange<const sint16>> elevations; // 8.8 fixed point real number format
	Holder<PointerRange<TileFlags>> tiles;

	uint32 index(ivec2 pos) const;
	uint32 index(vec2 pos) const;
	uint32 index(vec3 pos) const;
	ivec2 position(uint32 idx) const;
	vec3 center(uint32 idx) const;
	uint32 neighborDistance(uint32 a, uint32 b) const; // 24.8 format
};

struct Paths : private Immovable
{
	Holder<const Grid> grid;
	uint32 tile = m;

	Holder<PointerRange<uint32>> paths; // -1 = no path
	Holder<PointerRange<uint32>> distances; // 24.8 format

	void update();
};

struct MultiPaths : private Immovable
{
	Holder<PointerRange<Holder<Paths>>> paths;

	void update();

	struct FindResult
	{
		uint32 tile = m;
		uint32 distance = m;
	};

	FindResult find(uint32 startingPosition, uint32 visitedWaypointsBits) const;
};

extern Holder<Grid> globalGrid;
extern Holder<MultiPaths> globalPaths;

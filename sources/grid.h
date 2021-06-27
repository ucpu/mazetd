#include <cage-core/math.h>

using namespace cage;

enum class TileFlags : uint8
{
	None = 0,
	Invalid = 1u << 0, // tile is outside playable area
	Water = 1u << 1,
	Wall = 1u << 6,
	Spawn = 1u << 7,
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
	uint32 neighborDistance(uint32 a, uint32 b) const; // 16.16 format
};

extern Holder<Grid> globalGrid;

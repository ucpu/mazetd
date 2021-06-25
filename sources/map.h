#include <cage-core/math.h>

using namespace cage;

struct Map : private Immovable
{

};

enum class TileBlockingFlags : uint8
{
	None = 0,
	Outside = 1u << 0, // the tile is outside playable area
	Terrain = 1u << 1, // the tile has impassable terrain
	Occupied = 1u << 2, // the tile is blocked by in-game object
};

struct Grid : private Immovable
{
	ivec2 centerOffset;
	uint32 width = 0;
	uint32 height = 0;

	Holder<PointerRange<const sint16>> elevations;
	Holder<PointerRange<TileBlockingFlags>> blocking;

	Holder<PointerRange<uint32>> paths; // -1 = no path
	Holder<PointerRange<uint32>> distances; // 16.16 fixed point real number

	uint32 index(ivec2 pos) const;
	ivec2 position(uint32 idx) const;
	uint32 neighborDistance(uint32 a, uint32 b) const; // 16.16 format
	Holder<Grid> copy(bool copyPaths = false) const;
};

void gridUpdatePaths(Grid *grid, uint32 targetTile); // recalculate paths and distances based on blocking flags

extern Holder<Map> globalMap;
extern Holder<Grid> globalGrid;

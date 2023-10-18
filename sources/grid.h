#ifndef mazetd_header_grid
#define mazetd_header_grid

#include "common.h"

#include <cage-core/enumBits.h>

namespace cage
{
	class Collider;
}

namespace mazetd
{
	enum class TileFlags : uint16
	{
		None = 0,
		Invalid = 1u << 0, // tile is outside playable area
		Border = 1u << 1, // invalid tile that has a valid neighbor
		Waypoint = 1u << 2, // passable tile that player may not build on
		Water = 1u << 5,
		Sun = 1u << 6,
		Wind = 1u << 7,
		Snow = 1u << 8,
		Mana = 1u << 13, // mana ready for player to harvest
		Building = 1u << 14, // walls, towers and other impassable structures built by the player
		Trap = 1u << 15, // passable structures built by the player
	};
}

namespace cage
{
	GCHL_ENUM_BITS(::mazetd::TileFlags);
}

namespace mazetd
{
	struct Grid : private Immovable
	{
		Vec2i gridOffset;
		Vec2i resolution;

		Holder<PointerRange<const sint16>> elevations;
		Holder<PointerRange<TileFlags>> flags;

		uint32 index(Vec2i pos) const;
		uint32 index(Vec2 pos) const;
		uint32 index(Vec3 pos) const;
		Vec2i position(uint32 idx) const;
		Vec3 center(uint32 idx) const;
		Vec3 up(uint32 idx) const;
		uint32 neighborDistance(uint32 a, uint32 b) const; // 24.8 format
	};

	// directions and distances to single tile from all other tiles
	struct Directions : private Immovable
	{
		Holder<const Grid> grid;
		uint32 tile = m;

		Holder<PointerRange<uint32>> tiles; // -1 = no path
		Holder<PointerRange<uint32>> distances; // 24.8 format

		void update();
	};

	struct Waypoints : private Immovable
	{
		struct Waypoint : public Directions
		{
			Holder<PointerRange<uint32>> fullPath; // optimal path through all waypoints for monsters spawned on this waypoint
			uint32 fullDistance = m; // total distance through all waypoints for monsters spawned on this waypoint - 24.8 format
		};

		Holder<PointerRange<Holder<Waypoint>>> waypoints;
		uint32 avgFullDistance = 0; // approximate number of tiles for each monster when spawning at random spawners
		uint32 minFullDistance = 0; // approximate number of tiles for monsters spawned at the spawner with shortest path
		uint32 minDistanceSpawner = m; // waypoint index with shortest full path

		void update();

		struct FindResult
		{
			uint32 tile = m;
			uint32 distance = m; // 24.8 format
		};

		FindResult find(uint32 currentPosition, uint32 visitedWaypointsBits) const;
	};

	extern Holder<Grid> globalGrid;
	extern Holder<Waypoints> globalWaypoints;
	extern Holder<Collider> globalCollider;
}

#endif // !mazetd_header_grid

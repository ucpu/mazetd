#include <cage-core/enumerate.h>
#include <cage-core/concurrent.h>
#include <cage-core/threadPool.h>
#include <cage-core/pointerRangeHolder.h>

#include "generate.h"

#include <vector>
#include <queue>
#include <algorithm>

namespace
{
	typedef std::pair<uint32, uint32> QueueNode; // distance, tile
	typedef std::priority_queue<QueueNode, std::vector<QueueNode>, std::greater<QueueNode>> QueueStructureBase;
	struct QueueStructure : public QueueStructureBase
	{
		std::vector<QueueNode> &under() { return this->c; }
	};
}

void Directions::update()
{
	CAGE_ASSERT(grid);
	if (!tiles)
	{
		PointerRangeHolder<uint32> vec;
		vec.resize(grid->flags.size());
		tiles = vec;
	}
	if (!distances)
	{
		PointerRangeHolder<uint32> vec;
		vec.resize(grid->flags.size());
		distances = vec;
	}

	for (uint32 &it : tiles)
		it = m;
	for (uint32 &it : distances)
		it = m;
	std::vector<bool> visited;
	visited.resize(grid->flags.size(), false);
	QueueStructure queue;
	queue.under().reserve(1000);
	queue.push({ 0, tile });
	distances[tile] = 0;

	while (!queue.empty())
	{
		const uint32 id = queue.top().second;
		queue.pop();
		if (visited[id])
			continue;
		visited[id] = true;
		const ivec2 mp = grid->position(id);
		const uint32 md = distances[id];
		for (const ivec2 off : { ivec2(-1, 0), ivec2(1, 0), ivec2(0, -1), ivec2(0, 1) })
		{
			const uint32 ni = grid->index(mp + off);
			if (ni == m)
				continue;
			constexpr TileFlags Blocking = TileFlags::Invalid | TileFlags::Building;
			if (any(grid->flags[ni] & Blocking))
				continue;
			const uint32 nd = md + grid->neighborDistance(id, ni);
			if (nd < distances[ni])
			{
				CAGE_ASSERT(!visited[ni]);
				distances[ni] = nd;
				tiles[ni] = id;
				queue.push({ nd, ni });
			}
		}
	}
}

namespace
{
	void directionsThreadEntry(Waypoints *waypoints, uint32 thrId, uint32)
	{
		waypoints->waypoints[thrId]->update();
	}

	void waypointThreadEntry(Waypoints *waypoints, uint32 thrId, uint32)
	{
		PointerRangeHolder<uint32> path;
		path.reserve(1000);
		uint32 waypointBits = 0;
		uint32 prev = m;
		uint32 distance = 0;
		uint32 tile = waypoints->waypoints[thrId]->tile;
		while (tile != m)
		{
			for (const auto &it : enumerate(waypoints->waypoints))
				if (it.get()->tile == tile)
					waypointBits |= 1u << it.index;
			path.push_back(tile);
			if (prev != m)
				distance += waypoints->waypoints[thrId]->grid->neighborDistance(prev, tile);
			prev = tile;
			tile = waypoints->find(tile, waypointBits).tile;
		}
		waypoints->waypoints[thrId]->fullPath = path;
		waypoints->waypoints[thrId]->fullDistance = distance;
	}
}

void Waypoints::update()
{
	if (waypoints.empty())
		return;
	CAGE_LOG_DEBUG(SeverityEnum::Info, "paths", "recomputing paths");
	static Holder<ThreadPool> threadPool;
	if (!threadPool || threadPool->threadsCount() != waypoints.size())
		threadPool = newThreadPool("paths_", waypoints.size());
	threadPool->function.bind<Waypoints *, &directionsThreadEntry>(this);
	threadPool->run();
	threadPool->function.bind<Waypoints *, &waypointThreadEntry>(this);
	threadPool->run();
	uint32 sum = 0;
	for (const auto &it : waypoints)
		sum += it->fullDistance;
	avgFullDistance = (stor(sum) / waypoints.size()).value;
}

Waypoints::FindResult Waypoints::find(uint32 currentPosition, uint32 visitedWaypointsBits) const
{
	std::vector<uint32> indices;
	indices.reserve(waypoints.size());
	for (uint32 i = 0; i < waypoints.size(); i++)
	{
		if ((visitedWaypointsBits & (1u << i)) == 0)
			indices.push_back(i);
	}
	if (indices.empty())
		return {};
	FindResult bestResult;
	do
	{
		uint32 pos = currentPosition;
		uint32 dist = 0;
		for (uint32 i : indices)
		{
			dist += waypoints[i]->distances[pos];
			pos = waypoints[i]->tile;
		}
		if (dist < bestResult.distance)
		{
			bestResult.distance = dist;
			bestResult.tile = waypoints[indices[0]]->tiles[currentPosition];
		}
	} while (std::next_permutation(indices.begin(), indices.end()));
	return bestResult;
}

namespace
{
	uint32 findSpawnPoint(const Grid *grid)
	{
		const uint32 total = numeric_cast<uint32>(grid->flags.size());
		while (true)
		{
			const uint32 i = randomRange(0u, total);
			if (none(grid->flags[i] & (TileFlags::Invalid | TileFlags::Waypoint)))
				return i;
		}
	}
}

Holder<Waypoints> newWaypoints(Holder<Grid> grid)
{
	Holder<Waypoints> mp = systemMemory().createHolder<Waypoints>();
	PointerRangeHolder<Holder<Waypoints::Waypoint>> vec;
	vec.reserve(4);
	for (uint32 i = 0; i < 4; i++)
	{
		Holder<Waypoints::Waypoint> p = systemMemory().createHolder<Waypoints::Waypoint>();
		p->grid = grid.share();
		p->tile = findSpawnPoint(+grid);
		grid->flags[p->tile] |= TileFlags::Waypoint;
		vec.push_back(std::move(p));
	}
	mp->waypoints = vec;
	mp->update();
	return mp;
}

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

void Paths::update()
{
	CAGE_ASSERT(grid);
	if (!paths)
	{
		PointerRangeHolder<uint32> vec;
		vec.resize(grid->tiles.size());
		paths = vec;
	}
	if (!distances)
	{
		PointerRangeHolder<uint32> vec;
		vec.resize(grid->tiles.size());
		distances = vec;
	}

	for (uint32 &it : paths)
		it = m;
	for (uint32 &it : distances)
		it = m;
	std::vector<bool> visited;
	visited.resize(grid->tiles.size(), false);
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
			constexpr TileFlags Blocking = TileFlags::Invalid | TileFlags::Wall;
			if (any(grid->tiles[ni] & Blocking))
				continue;
			const uint32 nd = md + grid->neighborDistance(id, ni);
			if (nd < distances[ni])
			{
				CAGE_ASSERT(!visited[ni]);
				distances[ni] = nd;
				paths[ni] = id;
				queue.push({ nd, ni });
			}
		}
	}
}

namespace
{
	void pathsThreadEntry(MultiPaths *paths, uint32 thrId, uint32)
	{
		paths->paths[thrId]->update();
	}
}

void MultiPaths::update()
{
	if (paths.empty())
		return;
	Holder<ThreadPool> threadPool = newThreadPool("paths_", paths.size());
	threadPool->function.bind<MultiPaths *, &pathsThreadEntry>(this);
	threadPool->run();
}

MultiPaths::FindResult MultiPaths::find(uint32 startingPosition, uint32 visitedWaypointsBits) const
{
	// todo
	return {};
}

namespace
{
	uint32 findSpawnPoint(const Grid *grid)
	{
		const uint32 total = numeric_cast<uint32>(grid->tiles.size());
		while (true)
		{
			const uint32 i = randomRange(0u, total);
			if (none(grid->tiles[i] & (TileFlags::Invalid | TileFlags::Spawn)))
				return i;
		}
	}
}

Holder<MultiPaths> newMultiPaths(Holder<Grid> grid)
{
	Holder<MultiPaths> mp = systemMemory().createHolder<MultiPaths>();
	PointerRangeHolder<Holder<Paths>> vec;
	vec.reserve(4);
	for (uint32 i = 0; i < 4; i++)
	{
		Holder<Paths> p = systemMemory().createHolder<Paths>();
		p->grid = grid.share();
		p->tile = findSpawnPoint(+grid);
		grid->tiles[p->tile] |= TileFlags::Spawn;
		vec.push_back(std::move(p));
	}
	mp->paths = vec;
	mp->update();
	return mp;
}

Holder<MultiPaths> globalPaths;

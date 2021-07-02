#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>
#include <cage-core/pointerRangeHolder.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	WindowEventListeners listeners;

	bool placingWallBlocksMonsters()
	{
		Holder<Grid> grid = systemMemory().createHolder<Grid>();
		grid->gridOffset = globalGrid->gridOffset;
		grid->resolution = globalGrid->resolution;
		grid->elevations = globalGrid->elevations.share();
		grid->flags = PointerRangeHolder<TileFlags>(globalGrid->flags.begin(), globalGrid->flags.end());
		grid->flags[playerCursorTile] |= TileFlags::Wall;
		Holder<Directions> paths = systemMemory().createHolder<Directions>();
		paths->grid = grid.share();
		paths->tile = globalWaypoints->waypoints[0]->tile;
		paths->update();
		for (const auto &p : globalWaypoints->waypoints)
			if (paths->distances[p->tile] == m)
				return true; // disconnected spawner
		bool blocked = false;
		entitiesVisitor(gameEntities(), [&](const PositionComponent &p, const MonsterComponent &) {
			if (paths->distances[p.tile] == m)
				blocked = true; // disconnected monster
		});
		return blocked;
	}

	void placeBuilding()
	{
		const uint32 tile = playerCursorTile;
		CAGE_ASSERT(tile != m);
		TileFlags &flags = globalGrid->flags[tile];
		if (any(flags & (TileFlags::Invalid | TileFlags::Waypoint | TileFlags::Wall | TileFlags::Water)))
			return;
		if (placingWallBlocksMonsters())
			return;

		flags |= TileFlags::Wall;
		globalWaypoints->update();

		Entity *e = gameEntities()->createUnique();
		e->value<PositionComponent>().tile = tile;
		e->value<WallComponent>();
		Entity *f = e->value<EngineComponent>().entity;
		CAGE_COMPONENT_ENGINE(Render, r, f);
		r.object = HashString("mazetd/buildings/wall.obj");
	}

	void clearBuilding()
	{
		const uint32 tile = playerCursorTile;
		CAGE_ASSERT(tile != m);
		TileFlags &flags = globalGrid->flags[tile];
		if (none(flags & TileFlags::Wall))
			return;

		flags &= ~TileFlags::Wall;
		globalWaypoints->update();

		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &p, const WallComponent &) {
			if (p.tile == tile)
				e->destroy();
		}, true);
	}

	bool mouseEvent(MouseButtonsFlags buttons, ModifiersFlags mods, const ivec2 &)
	{
		if (!gameRunning || playerCursorTile == m || mods != ModifiersFlags::None)
			return false;
		switch (buttons)
		{
		case MouseButtonsFlags::Left:
			placeBuilding();
			return true;
		case MouseButtonsFlags::Middle:
			clearBuilding();
			return true;
		}
		return false;
	}

	void engineInit()
	{
		listeners.attachAll(engineWindow());
		listeners.mousePress.bind<&mouseEvent>();
		listeners.mouseMove.bind<&mouseEvent>();
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize);
			engineInitListener.bind<&engineInit>();
		}
	} callbacksInstance;
}

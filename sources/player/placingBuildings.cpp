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

	bool placingBuildingBlocksMonsters()
	{
		Holder<Grid> grid = systemMemory().createHolder<Grid>();
		grid->gridOffset = globalGrid->gridOffset;
		grid->resolution = globalGrid->resolution;
		grid->elevations = globalGrid->elevations.share();
		grid->flags = PointerRangeHolder<TileFlags>(globalGrid->flags.begin(), globalGrid->flags.end());
		grid->flags[playerCursorTile] |= TileFlags::Building;
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

	constexpr TileFlags commonUnbuildable = TileFlags::Invalid | TileFlags::Waypoint | TileFlags::Building | TileFlags::Trap;

	bool canPlaceBuilding()
	{
		const TileFlags flags = globalGrid->flags[playerCursorTile];
		if (any(flags & (commonUnbuildable | TileFlags::Water | TileFlags::Snow)))
			return false;
		if (placingBuildingBlocksMonsters())
			return false;
		return true;
	}

	bool canPlaceTrap()
	{
		const TileFlags flags = globalGrid->flags[playerCursorTile];
		return none(flags & commonUnbuildable);
	}

	bool selectionIsTrap()
	{
		return playerBuildingSelection >= 1300;
	}

	void placeTile()
	{
		TileFlags &flags = globalGrid->flags[playerCursorTile];

		if (selectionIsTrap())
		{
			if (!canPlaceTrap())
				return;

			// todo take some money

			flags |= TileFlags::Trap;

			Entity *e = gameEntities()->createUnique();
			e->value<PositionComponent>().tile = playerCursorTile;
			e->value<TrapComponent>();
			Entity *f = e->value<EngineComponent>().entity;
			CAGE_COMPONENT_ENGINE(Transform, t, f);
			t.scale = 0.3;
			CAGE_COMPONENT_ENGINE(Render, r, f);
			r.object = HashString("cage/model/fake.obj");
		}
		else
		{
			if (!canPlaceBuilding())
				return;

			// todo take some money

			flags |= TileFlags::Building;
			globalWaypoints->update();

			Entity *e = gameEntities()->createUnique();
			e->value<PositionComponent>().tile = playerCursorTile;
			e->value<BuildingComponent>();
			Entity *f = e->value<EngineComponent>().entity;
			CAGE_COMPONENT_ENGINE(Render, r, f);
			r.object = HashString("mazetd/buildings/wall.obj");
		}
	}

	void clearTile()
	{
		TileFlags &flags = globalGrid->flags[playerCursorTile];

		if (any(flags & TileFlags::Trap))
		{
			flags &= ~TileFlags::Trap;

			entitiesVisitor(gameEntities(), [](Entity *e, const PositionComponent &p, const TrapComponent &) {
				if (p.tile == playerCursorTile)
				{
					// todo return some money
					e->destroy();
				}
				}, true);
		}
		
		if (any(flags & TileFlags::Building))
		{
			flags &= ~TileFlags::Building;
			globalWaypoints->update();

			entitiesVisitor(gameEntities(), [](Entity *e, const PositionComponent &p, const BuildingComponent &) {
				if (p.tile == playerCursorTile)
				{
					// todo return some money
					e->destroy();
				}
			}, true);
		}
	}

	bool mouseEvent(MouseButtonsFlags buttons, ModifiersFlags mods, const ivec2 &)
	{
		if (!gameRunning || playerCursorTile == m || mods != ModifiersFlags::None)
			return false;
		switch (buttons)
		{
		case MouseButtonsFlags::Left:
			placeTile();
			return true;
		case MouseButtonsFlags::Middle:
			clearTile();
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

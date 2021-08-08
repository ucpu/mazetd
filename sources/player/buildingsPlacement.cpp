#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>
#include <cage-core/pointerRangeHolder.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

void destroyShortestPathVisualizationMarks();
void spatialUpdateStructures();

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
		entitiesVisitor([&](const PositionComponent &p, const MonsterComponent &) {
			if (paths->distances[p.tile] == m)
				blocked = true; // disconnected monster
			}, gameEntities(), false);
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

	template<class T>
	void copyComp(Entity *e)
	{
		if (playerBuildingSelection->has<T>())
			e->value<T>() = playerBuildingSelection->value<T>();
	}

	void placeStructure()
	{
		if (playerBuildingSelection->has<TrapComponent>())
		{
			if (!canPlaceTrap())
				return;
		}
		else
		{
			if (!canPlaceBuilding())
				return;
		}

		const uint32 moneyCost = playerBuildingSelection->value<CostComponent>().cost;
		CAGE_ASSERT(moneyCost != m);
		if (playerMoney < moneyCost)
			return;
		playerMoney -= moneyCost;

		Entity *e = gameEntities()->createUnique();
		e->value<PositionComponent>().tile = playerCursorTile;

		copyComp<NameComponent>(e);
		copyComp<BuildingComponent>(e);
		copyComp<TrapComponent>(e);
		copyComp<CostComponent>(e);
		copyComp<PivotComponent>(e);
		copyComp<DamageComponent>(e);
		copyComp<ModBonusComponent>(e);
		copyComp<ModTargetingComponent>(e);
		copyComp<ModElementComponent>(e);
		copyComp<ManaStorageComponent>(e);
		copyComp<ManaDistributorComponent>(e);
		copyComp<ManaReceiverComponent>(e);
		copyComp<ManaCollectorComponent>(e);

		{
			Entity *f = e->value<EngineComponent>().entity;
			RenderComponent &r = f->value<RenderComponent>();
			r.object = playerBuildingSelection->value<GuiModelComponent>().model;
			TextureAnimationComponent &ta = f->value<TextureAnimationComponent>();
			ta.startTime = randomRange(0u, 1000000000u);
		}

		if (playerBuildingSelection->has<TrapComponent>())
		{
			e->value<TrapComponent>();
			globalGrid->flags[playerCursorTile] |= TileFlags::Trap;
		}
		else
		{
			e->value<BuildingComponent>();
			globalGrid->flags[playerCursorTile] |= TileFlags::Building;
			globalWaypoints->update();
		}
		destroyShortestPathVisualizationMarks();
		spatialUpdateStructures();
	}

	void clearStructure()
	{
		TileFlags &flags = globalGrid->flags[playerCursorTile];

		if (any(flags & TileFlags::Trap))
		{
			flags &= ~TileFlags::Trap;

			entitiesVisitor([](Entity *e, const PositionComponent &p, const TrapComponent &) {
				if (p.tile == playerCursorTile)
				{
					playerMoney += 8 * e->value<CostComponent>().cost / 10;
					e->destroy();
				}
				}, gameEntities(), true);
		}

		if (any(flags & TileFlags::Building))
		{
			flags &= ~TileFlags::Building;
			globalWaypoints->update();

			entitiesVisitor([](Entity *e, const PositionComponent &p, const BuildingComponent &) {
				if (p.tile == playerCursorTile)
				{
					playerMoney += 8 * e->value<CostComponent>().cost / 10;
					e->destroy();
				}
				}, gameEntities(), true);
		}

		destroyShortestPathVisualizationMarks();
		spatialUpdateStructures();
	}

	bool mouseEvent(MouseButtonsFlags buttons, ModifiersFlags mods, const ivec2 &)
	{
		if (!gameRunning || playerCursorTile == m || mods != ModifiersFlags::None)
			return false;
		switch (buttons)
		{
		case MouseButtonsFlags::Left:
			placeStructure();
			return true;
		case MouseButtonsFlags::Middle:
			clearStructure();
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

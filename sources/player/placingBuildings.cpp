#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>
#include <cage-core/pointerRangeHolder.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

void destroyShortestPathVisualizationMarks();

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

	uint32 structureModelName()
	{
		switch (playerBuildingSelection)
		{
		case 900: return HashString("mazetd/buildings/wall.object");
		case 1000: return HashString("mazetd/buildings/cheap.object");
		case 1001: return HashString("mazetd/buildings/fast.object");
		case 1002: return HashString("mazetd/buildings/heavy.object");
		case 1003: return HashString("mazetd/buildings/splash.object");
		case 1004: return HashString("mazetd/buildings/sniper.object");
		case 1100: return HashString("mazetd/buildings/augment-fire.object");
		case 1101: return HashString("mazetd/buildings/augment-water.object");
		case 1102: return HashString("mazetd/buildings/augment-poison.object");
		case 1204: return HashString("mazetd/buildings/mana-relay.object");
		case 1205: return HashString("mazetd/buildings/mana-capacitor.object");
		case 1300: return HashString("mazetd/buildings/trap-spikes.object");
		case 1301: return HashString("mazetd/buildings/trap-slow.object");
		case 1302: return HashString("mazetd/buildings/trap-haste.object");
		default: return HashString("cage/model/fake.obj");
		}
	}

	string structureDisplayName()
	{
		switch (playerBuildingSelection)
		{
		case 900: return  "Wall";
		case 1000: return "Cheap Tower";
		case 1001: return "Fast Tower";
		case 1002: return "Heavy Tower";
		case 1003: return "Splash Tower";
		case 1004: return "Sniper Tower";
		case 1100: return "Fire Augment";
		case 1101: return "Water Augment";
		case 1102: return "Poison Augment";
		case 1200: return "Waterwheel Collector";
		case 1201: return "Sunbloom Collector";
		case 1202: return "Windmill Collector";
		case 1203: return "Snowmill Collector";
		case 1204: return "Mana Relay";
		case 1205: return "Mana Capacitor";
		case 1300: return "Spikes Trap";
		case 1301: return "Slowing Trap";
		case 1302: return "Hastening Trap";
		default: return "<unknown building>";
		}
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
		}
		else
		{
			if (!canPlaceBuilding())
				return;

			// todo take some money

			flags |= TileFlags::Building;
			globalWaypoints->update();
		}

		Entity *e = gameEntities()->createUnique();
		e->value<PositionComponent>().tile = playerCursorTile;
		e->value<NameComponent>().name = structureDisplayName();

		if (selectionIsTrap())
			e->value<TrapComponent>();
		else
			e->value<BuildingComponent>();

		switch (playerBuildingSelection)
		{
		case 1000: // cheap
		{
			e->value<PivotComponent>().elevation = 1.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.damage /= 2;
			a.useAugments = false;
		} break;
		case 1001: // fast
		{
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod /= 3;
		} break;
		case 1002: // heavy
		{
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.damage *= 6;
			a.firingPeriod *= 2;
			a.splashRadius += 0.5;
		} break;
		case 1003: // splash
		{
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.splashRadius += 4;
		} break;
		case 1004: // sniper
		{
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.damage *= 3;
			a.firingPeriod *= 3;
			a.firingRange *= 3;
		} break;
		case 1100: // fire augment
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeEnum::Fire;
			e->value<ManaStorageComponent>().capacity = 100;
			e->value<ManaReceiverComponent>();
		} break;
		case 1101: // water augment
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeEnum::Water;
			e->value<ManaStorageComponent>().capacity = 100;
			e->value<ManaReceiverComponent>();
		} break;
		case 1102: // poison augment
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeEnum::Poison;
			e->value<ManaStorageComponent>().capacity = 100;
			e->value<ManaReceiverComponent>();
		} break;
		case 1200: // water collector
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Water;
			e->value<ManaDistributorComponent>();
		} break;
		case 1201: // sun collector
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Sun;
			e->value<ManaDistributorComponent>();
		} break;
		case 1202: // wind collector
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Wind;
			e->value<ManaDistributorComponent>();
		} break;
		case 1203: // snow collector
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Snow;
			e->value<ManaDistributorComponent>();
		} break;
		case 1204: // mana relay
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>().capacity = 10;
			e->value<ManaDistributorComponent>().range = 10;
			e->value<ManaReceiverComponent>();
		} break;
		case 1205: // mana capacitor
		{
			e->value<PivotComponent>().elevation = 1.5;
			e->value<ManaStorageComponent>().capacity = 1000;
			e->value<ManaDistributorComponent>().transferLimit = 50;
			e->value<ManaReceiverComponent>();
		} break;
		}

		{
			Entity *f = e->value<EngineComponent>().entity;
			CAGE_COMPONENT_ENGINE(Render, r, f);
			r.object = structureModelName();
			CAGE_COMPONENT_ENGINE(TextureAnimation, ta, f);
			ta.startTime = randomRange(0u, 1000000000u);
		}

		destroyShortestPathVisualizationMarks();
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

		destroyShortestPathVisualizationMarks();
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

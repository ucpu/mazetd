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
		case 1002: return HashString("mazetd/buildings/splash.object");
		case 1003: return HashString("mazetd/buildings/sniper.object");
		case 1100: return HashString("mazetd/buildings/augment-fire.object");
		case 1101: return HashString("mazetd/buildings/augment-water.object");
		case 1102: return HashString("mazetd/buildings/augment-poison.object");
		case 1200: return HashString("mazetd/buildings/mage.object");
		case 1201: return HashString("mazetd/buildings/collector-water.object");
		case 1202: return HashString("mazetd/buildings/collector-sun.object");
		case 1203: return HashString("mazetd/buildings/collector-wind.object");
		case 1204: return HashString("mazetd/buildings/collector-snow.object");
		case 1205: return HashString("mazetd/buildings/mana-relay.object");
		case 1206: return HashString("mazetd/buildings/mana-capacitor.object");
		case 1300: return HashString("mazetd/buildings/trap-spikes.object");
		case 1301: return HashString("mazetd/buildings/trap-slow.object");
		case 1302: return HashString("mazetd/buildings/trap-haste.object");
		default: return HashString("cage/model/fake.obj");
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

		if (selectionIsTrap())
			e->value<TrapComponent>();
		else
			e->value<BuildingComponent>();

		switch (playerBuildingSelection)
		{
		case 900: // wall
		{
			e->value<NameComponent>().name = "Wall";
		} break;
		case 1000: // cheap
		{
			e->value<NameComponent>().name = "Cheap Tower";
			e->value<PivotComponent>().elevation = 1.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 30;
			a.firingRange = 5;
			a.damage = 5;
			a.damageType = DamageTypeFlags::Physical;
			a.effectType = EffectTypeEnum::Physical;
		} break;
		case 1001: // fast
		{
			e->value<NameComponent>().name = "Fast Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 10;
			a.firingRange = 5;
			a.damage = 10;
			a.damageType = DamageTypeFlags::Physical;
			a.effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
		} break;
		case 1002: // splash
		{
			e->value<NameComponent>().name = "Splash Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 30;
			a.firingRange = 5;
			a.splashRadius = 3;
			a.damage = 10;
			a.damageType = DamageTypeFlags::Physical;
			a.effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
		} break;
		case 1003: // sniper
		{
			e->value<NameComponent>().name = "Sniper Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 90;
			a.firingRange = 15;
			a.damage = 30;
			a.damageType = DamageTypeFlags::Physical;
			a.effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
		} break;
		case 1100: // fire augment
		{
			e->value<NameComponent>().name = "Fire Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeFlags::Fire;
			e->value<AugmentComponent>().effectType = EffectTypeEnum::Fire;
		} break;
		case 1101: // water augment
		{
			e->value<NameComponent>().name = "Water Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeFlags::Water;
			e->value<AugmentComponent>().effectType = EffectTypeEnum::Water;
		} break;
		case 1102: // poison augment
		{
			e->value<NameComponent>().name = "Poison Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().damageType = DamageTypeFlags::Poison;
			e->value<AugmentComponent>().effectType = EffectTypeEnum::Poison;
		} break;
		case 1200: // mage
		{
			e->value<NameComponent>().name = "Mage Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 30;
			a.firingRange = 6;
			a.splashRadius = 1.5;
			a.damage = 100;
			a.manaCost = 30;
			a.damageType = DamageTypeFlags::Magic;
			a.effectType = EffectTypeEnum::Mana;
			a.useAugments = false;
			e->value<ManaStorageComponent>();
			e->value<ManaReceiverComponent>();
		} break;
		case 1201: // water collector
		{
			e->value<NameComponent>().name = "Waterwheel Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Water;
			e->value<ManaCollectorComponent>().collectAmount = 20;
			e->value<ManaDistributorComponent>();
		} break;
		case 1202: // sun collector
		{
			e->value<NameComponent>().name = "Sunbloom Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Sun;
			e->value<ManaCollectorComponent>().collectAmount = 10;
			e->value<ManaDistributorComponent>();
		} break;
		case 1203: // wind collector
		{
			e->value<NameComponent>().name = "Windmill Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Wind;
			e->value<ManaCollectorComponent>().collectAmount = 10;
			e->value<ManaDistributorComponent>();
		} break;
		case 1204: // snow collector
		{
			e->value<NameComponent>().name = "Snowmill Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Snow;
			e->value<ManaCollectorComponent>().collectAmount = 20;
			e->value<ManaDistributorComponent>();
		} break;
		case 1205: // mana relay
		{
			e->value<NameComponent>().name = "Mana Relay";
			e->value<PivotComponent>().elevation = 2.5;
			e->value<ManaStorageComponent>();
			e->value<ManaDistributorComponent>().range = 10;
			e->value<ManaReceiverComponent>();
		} break;
		case 1206: // mana capacitor
		{
			e->value<NameComponent>().name = "Mana Capacitor";
			e->value<PivotComponent>().elevation = 1.3;
			e->value<ManaStorageComponent>().capacity = 5000;
			e->value<ManaDistributorComponent>();
			e->value<ManaReceiverComponent>();
		} break;
		case 1300: // spikes trap
		{
			e->value<NameComponent>().name = "Spikes Trap";
			e->value<PivotComponent>().elevation = 0.2;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 5;
			a.firingRange = 0.5;
			a.damage = 2;
			a.damageType = DamageTypeFlags::Physical;
			a.effectType = EffectTypeEnum::Physical;
			a.targetClasses &= ~MonsterClassFlags::Flyer;
		} break;
		case 1301: // slow trap
		{
			e->value<NameComponent>().name = "Slowing Trap";
			e->value<PivotComponent>().elevation = 0.2;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 5;
			a.firingRange = 0.5;
			a.damageType = DamageTypeFlags::Slow;
			a.targetClasses &= ~MonsterClassFlags::Flyer;
		} break;
		case 1302: // haste trap
		{
			e->value<NameComponent>().name = "Hastening Trap";
			e->value<PivotComponent>().elevation = 0.2;
			AttackComponent &a = e->value<AttackComponent>();
			a.firingPeriod = 5;
			a.firingRange = 0.5;
			a.damageType = DamageTypeFlags::Haste;
			a.targetClasses &= ~MonsterClassFlags::Flyer;
		} break;
		default:
		{
			e->value<NameComponent>().name = "<unknown>";
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

#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>
#include <cage-core/pointerRangeHolder.h>
#include <cage-engine/engine.h>
#include <cage-engine/window.h>

#include "../game.h"
#include "../grid.h"

void destroyShortestPathVisualizationMarks();

uint32 structureMoneyCost(uint32 id)
{
	switch (id)
	{
	case 900: return 5; // wall
	case 1000: return 40; // cheap // 1 Dps = 4 $
	case 1001: return 150; // fast // 1 Dps = 5 $ / augment: 1 Dps = 0.25 #ps = 1.25 $ / total: 1 Dps = 3 $
	case 1002: return 150; // splash // 1 Dps = 15 $ / augment: 1 Dps = 0.75 #ps = 3.75 $
	case 1003: return 150; // sniper // 1 Dps = 7.5 $ / augment: 1 Dps = 0.375 #ps = 1.875 $
	case 1004: return 200; // mage // 1 Dps = 0.2 #ps = 2 $ / total: 1 Dps = 3 $
	case 1100: return 500; // fire augment
	case 1101: return 500; // water augment
	case 1102: return 500; // poison augment
	case 1200: return 300; // water collector
	case 1201: return 300; // sun collector // (produces: 120 #ps)
	case 1202: return 300; // wind collector
	case 1203: return 300; // snow collector
	case 1204: return 100; // mana relay // (transfers: 500 #ps)
	case 1205: return 1500; // mana capacitor
	case 1300: return 40; // spikes trap // 1 Dps / 3.333 $
	case 1301: return 300; // slow trap
	case 1302: return 200; // haste trap
	default: return m;
	}
}

void AttackComponent::initAugmentData()
{
	for (AugmentEnum a : { AugmentEnum::Fire, AugmentEnum::Water, AugmentEnum::Poison })
	{
		data[(int)a] = data[0];
		data[(int)a].damage *= 4;
		data[(int)a].manaCost = data[0].firingPeriod; // 30 mana per second
	}
	data[(int)AugmentEnum::Fire].damageType = DamageTypeFlags::Fire;
	data[(int)AugmentEnum::Fire].effectType = EffectTypeEnum::Fire;
	data[(int)AugmentEnum::Water].damageType = DamageTypeFlags::Water;
	data[(int)AugmentEnum::Water].effectType = EffectTypeEnum::Water;
	data[(int)AugmentEnum::Poison].damageType = DamageTypeFlags::Poison;
	data[(int)AugmentEnum::Poison].effectType = EffectTypeEnum::Poison;
}

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
		case 1004: return HashString("mazetd/buildings/mage.object");
		case 1100: return HashString("mazetd/buildings/augment-fire.object");
		case 1101: return HashString("mazetd/buildings/augment-water.object");
		case 1102: return HashString("mazetd/buildings/augment-poison.object");
		case 1200: return HashString("mazetd/buildings/collector-water.object");
		case 1201: return HashString("mazetd/buildings/collector-sun.object");
		case 1202: return HashString("mazetd/buildings/collector-wind.object");
		case 1203: return HashString("mazetd/buildings/collector-snow.object");
		case 1204: return HashString("mazetd/buildings/mana-relay.object");
		case 1205: return HashString("mazetd/buildings/mana-capacitor.object");
		case 1300: return HashString("mazetd/buildings/trap-spikes.object");
		case 1301: return HashString("mazetd/buildings/trap-slow.object");
		case 1302: return HashString("mazetd/buildings/trap-haste.object");
		default: return HashString("cage/model/fake.obj");
		}
	}

	void placeTile()
	{
		if (selectionIsTrap())
		{
			if (!canPlaceTrap())
				return;
		}
		else
		{
			if (!canPlaceBuilding())
				return;
		}

		const uint32 moneyCost = structureMoneyCost(playerBuildingSelection);
		CAGE_ASSERT(moneyCost != m);
		if (playerMoney < moneyCost)
			return;
		playerMoney -= moneyCost;

		Entity *e = gameEntities()->createUnique();
		e->value<PositionComponent>().tile = playerCursorTile;
		e->value<RefundCostComponent>().cost = 9 * moneyCost / 10;

		if (selectionIsTrap())
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
			a.data[0].firingPeriod = 30;
			a.data[0].firingRange = 5;
			a.data[0].damage = 10;
			a.data[0].damageType = DamageTypeFlags::Physical;
			a.data[0].effectType = EffectTypeEnum::Physical;
			a.useAugments = false;
		} break;
		case 1001: // fast
		{
			e->value<NameComponent>().name = "Fast Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 10;
			a.data[0].firingRange = 5;
			a.data[0].damage = 10;
			a.data[0].damageType = DamageTypeFlags::Physical;
			a.data[0].effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
			a.initAugmentData();
		} break;
		case 1002: // splash
		{
			e->value<NameComponent>().name = "Splash Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 30;
			a.data[0].firingRange = 5;
			a.data[0].splashRadius = 3;
			a.data[0].damage = 10;
			a.data[0].damageType = DamageTypeFlags::Physical;
			a.data[0].effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
			a.initAugmentData();
		} break;
		case 1003: // sniper
		{
			e->value<NameComponent>().name = "Sniper Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 90;
			a.data[0].firingRange = 15;
			a.data[0].damage = 60;
			a.data[0].damageType = DamageTypeFlags::Physical;
			a.data[0].effectType = EffectTypeEnum::Physical;
			a.useAugments = true;
			a.initAugmentData();
		} break;
		case 1004: // mage
		{
			e->value<NameComponent>().name = "Mage Tower";
			e->value<PivotComponent>().elevation = 2.5;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 30;
			a.data[0].firingRange = 5;
			a.data[0].damage = 100;
			a.data[0].manaCost = 20;
			a.data[0].damageType = DamageTypeFlags::Magic;
			a.data[0].effectType = EffectTypeEnum::Mana;
			a.useAugments = false;
			e->value<ManaStorageComponent>();
			e->value<ManaReceiverComponent>();
		} break;
		case 1100: // fire augment
		{
			e->value<NameComponent>().name = "Fire Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().data = AugmentEnum::Fire;
		} break;
		case 1101: // water augment
		{
			e->value<NameComponent>().name = "Water Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().data = AugmentEnum::Water;
		} break;
		case 1102: // poison augment
		{
			e->value<NameComponent>().name = "Poison Augment";
			e->value<PivotComponent>().elevation = 1.5;
			e->value<AugmentComponent>().data = AugmentEnum::Poison;
		} break;
		case 1200: // water collector
		{
			e->value<NameComponent>().name = "Waterwheel Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Water;
			e->value<ManaCollectorComponent>().collectAmount = 20;
			e->value<ManaDistributorComponent>();
		} break;
		case 1201: // sun collector
		{
			e->value<NameComponent>().name = "Sunbloom Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Sun;
			e->value<ManaCollectorComponent>().collectAmount = 10;
			e->value<ManaDistributorComponent>();
		} break;
		case 1202: // wind collector
		{
			e->value<NameComponent>().name = "Windmill Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Wind;
			e->value<ManaCollectorComponent>().collectAmount = 10;
			e->value<ManaDistributorComponent>();
		} break;
		case 1203: // snow collector
		{
			e->value<NameComponent>().name = "Snowmelt Collector";
			e->value<PivotComponent>().elevation = 1;
			e->value<ManaStorageComponent>();
			e->value<ManaCollectorComponent>().type = ManaCollectorTypeEnum::Snow;
			e->value<ManaCollectorComponent>().collectAmount = 20;
			e->value<ManaDistributorComponent>();
		} break;
		case 1204: // mana relay
		{
			e->value<NameComponent>().name = "Mana Relay";
			e->value<PivotComponent>().elevation = 2.5;
			e->value<ManaStorageComponent>();
			e->value<ManaDistributorComponent>().range = 10;
			e->value<ManaReceiverComponent>();
		} break;
		case 1205: // mana capacitor
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
			a.data[0].firingPeriod = 5;
			a.data[0].firingRange = 0.5;
			a.data[0].damage = 2;
			a.data[0].damageType = DamageTypeFlags::Physical;
			a.data[0].effectType = EffectTypeEnum::Physical;
			a.data[0].invalidClasses = MonsterClassFlags::Flyer;
			a.useAugments = false;
		} break;
		case 1301: // slow trap
		{
			e->value<NameComponent>().name = "Slowing Trap";
			e->value<PivotComponent>().elevation = 0.2;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 5;
			a.data[0].firingRange = 0.5;
			a.data[0].damageType = DamageTypeFlags::Slow;
			a.data[0].invalidClasses = MonsterClassFlags::Flyer;
			a.useAugments = false;
		} break;
		case 1302: // haste trap
		{
			e->value<NameComponent>().name = "Hastening Trap";
			e->value<PivotComponent>().elevation = 0.2;
			AttackComponent &a = e->value<AttackComponent>();
			a.data[0].firingPeriod = 5;
			a.data[0].firingRange = 0.5;
			a.data[0].damageType = DamageTypeFlags::Haste;
			a.data[0].invalidClasses = MonsterClassFlags::Flyer;
			a.useAugments = false;
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
					playerMoney += e->value<RefundCostComponent>().cost;
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
					playerMoney += e->value<RefundCostComponent>().cost;
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

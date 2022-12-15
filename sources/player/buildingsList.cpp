#include <cage-core/hashString.h>
#include <cage-engine/guiComponents.h>
#include <cage-engine/guiManager.h>
#include <cage-simple/engine.h>

#include "../game.h"

namespace
{
	bool buildingSelectionClick(uint32 i)
	{
		playerBuildingSelection = engineGuiEntities()->get(i);
		engineGuiManager()->focus(0); // defocus to allow using keyboard shortcuts
		return true;
	}

	void buildingTooltip(const GuiTooltipConfig &config)
	{
		Entity *sb = config.invoker;
		if (!sb)
			return;

		EntityManager *ents = engineGuiEntities();

		{
			Entity *e = config.tooltip;
			e->value<GuiPanelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + sb->value<NameComponent>().name;
			e->value<GuiLayoutLineComponent>().vertical = true;
		}

		sint32 index = 0;

		{ // description
			CAGE_ASSERT(sb->has<DescriptionComponent>());
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = config.tooltip->name();
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = String(sb->value<DescriptionComponent>().description);
		}

		// damage
		if (sb->has<DamageComponent>())
		{
			{ // dps
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Damage per second: " + (30.f * sb->value<DamageComponent>().damage / sb->value<DamageComponent>().firingPeriod);
			}

			{ // firing range
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Firing range: " + sb->value<DamageComponent>().firingRange;
			}

			// splash radius
			if (sb->value<DamageComponent>().splashRadius > 0)
			{
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Splash radius: " + sb->value<DamageComponent>().splashRadius;
			}

			// mana per second
			if (sb->value<DamageComponent>().manaCost > 0)
			{
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Mana use per second: " + (30.f * sb->value<DamageComponent>().manaCost / sb->value<DamageComponent>().firingPeriod);
			}
		}

		// mana collector
		if (sb->has<ManaCollectorComponent>())
		{
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = config.tooltip->name();
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Harvested mana multiplier: " + sb->value<ManaCollectorComponent>().collectAmount;
		}

		// mana storage
		if (sb->has<ManaStorageComponent>())
		{
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = config.tooltip->name();
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Mana capacity: " + sb->value<ManaStorageComponent>().capacity;
		}

		// mana distributor
		if (sb->has<ManaDistributorComponent>())
		{
			{ // transfer limit
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Mana transfer rate: " + sb->value<ManaDistributorComponent>().transferLimit;
			}
			{ // transfer range
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = config.tooltip->name();
				pp.order = index++;
				e->value<GuiLabelComponent>();
				e->value<GuiTextComponent>().value = Stringizer() + "Mana transfer range: " + sb->value<ManaDistributorComponent>().range;
			}
		}

		{ // cost
			CAGE_ASSERT(sb->has<CostComponent>());
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = config.tooltip->name();
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Cost: " + sb->value<CostComponent>().cost;
		}
	}

	struct BuildingsGenerator
	{
		EntityManager *ents = engineGuiEntities();
		uint32 index = 1000;

		static constexpr StringPointer categoryNames[] = { "Damage", "Enhancements", "Targeting", "Elements", "Mana" };

		void prepareCategories()
		{
			sint32 order = 0;
			for (StringPointer name : categoryNames)
			{
				Entity *e = ents->create(410 + order++);
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 401;
				pp.order = order;
				e->value<GuiSpoilerComponent>();
				e->value<GuiLayoutLineComponent>().vertical = true;
				e->value<GuiTextComponent>().value = String(name);
			}
		}

		Entity *generateBase(StringPointer name)
		{
			Entity *e = ents->create(++index);
			e->value<GuiButtonComponent>();
			e->value<GuiTextComponent>().value = String(name);
			e->value<GuiEventComponent>().event.bind<&buildingSelectionClick>();
			e->value<NameComponent>().name = name;
			e->value<GuiTooltipComponent>().tooltip.bind<&buildingTooltip>();
			return e;
		}

		Entity *generate(uint32 categoryIndex, StringPointer name)
		{
			Entity *e = generateBase(name);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 410 + categoryIndex;
			pp.order = index;
			return e;
		}

		void generateAll()
		{
			// wall

			{
				Entity *e = generateBase("Wall");
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 401;
				pp.order = -1;
				e->value<BuildingComponent>();
				e->value<CostComponent>().cost = 5;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/wall.object");
				e->value<DescriptionComponent>().description = "Blocks monsters path, but does not attack.";
			}

			// damage

			{
				Entity *e = generate(0, "Spikes");
				e->value<TrapComponent>();
				e->value<CostComponent>().cost = 10;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingRange = 0.6;
				d.firingPeriod = 10; // 3 shots per second
				d.damage = 1;
				d.overTime = 6;
				d.invalidClasses = MonsterClassFlags::Flier;
				d.acceptMods = false;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/trap-spikes.object");
				e->value<DescriptionComponent>().description = "Placed in monsters way, damages monsters walking over it.\nCannot damage flying monsters.\nCannot use any enhancements, targeting, or elements.";
				// dps: 3
				// dps per dollar: 0.3
			}

			{
				Entity *e = generate(0, "Arrow Tower");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.1;
				e->value<CostComponent>().cost = 200;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingRange = 5;
				d.firingPeriod = 30; // 1 shots per second
				d.damage = 30;
				d.manaCost = 10;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-light.object");
				e->value<DescriptionComponent>().description = "Most common and cheapest tower. It has good mana efficiency.\nRequires mana to shoot.";
				e->value<ManaStorageComponent>().capacity = 100;
				e->value<ManaReceiverComponent>();
				// dps: 30
				// dps per dollar: 0.15
				// damage per mana: 3
				// mana per second: 10

				// balance:
				// 50 * arrow tower, 5 * enhancement, 5 * element, 10 * collector, 20 * relay, 5 * capacitor
				// 12'000 dps, 500 mana/s, 26'500 money
			}

			{
				Entity *e = generate(0, "Sniper Tower");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.8;
				e->value<CostComponent>().cost = 300;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingRange = 8;
				d.firingPeriod = 120; // 0.25 shots per second
				d.damage = 180;
				d.manaCost = 90;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-heavy.object");
				e->value<DescriptionComponent>().description = "Large effective radius provides for tactical use.\nRequires mana to shoot.";
				e->value<ManaStorageComponent>().capacity = 400;
				e->value<ManaReceiverComponent>();
				// dps: 45
				// dps per dollar: 0.15
				// damage per mana: 2
				// mana per second: 22.5
			}

			{
				Entity *e = generate(0, "Bombard Tower");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.6;
				e->value<CostComponent>().cost = 600;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingRange = 4;
				d.splashRadius = 2;
				d.firingPeriod = 30; // 1 shots per second
				d.damage = 30;
				d.manaCost = 45;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-medium.object");
				e->value<DescriptionComponent>().description = "Exploding shells deal damage to surrounding monsters around the initial target.\nRequires mana to shoot.";
				e->value<ManaStorageComponent>().capacity = 300;
				e->value<ManaReceiverComponent>();
				// dps: 30 (effective 90)
				// dps per dollar: 0.05 (effective 0.15)
				// damage per mana: 0.666 (effective 2)
				// mana per second: 45
			}

			// enhancements

			struct EnhancementData
			{
				StringPointer name;
				StringPointer description;
				EnhancementTypeEnum enhancement = EnhancementTypeEnum::None;
				uint32 model = 0;
			};
			static constexpr EnhancementData enhancementData[] = {
				{ "Damage", "Affected towers have doubled damage and mana cost.\nEnhancements do not stack.", EnhancementTypeEnum::Damage, HashString("mazetd/buildings/bonus-damage.object")},
				{ "Firing Rate", "Affected towers have doubled rate of fire.\nEnhancements do not stack.", EnhancementTypeEnum::FiringRate, HashString("mazetd/buildings/bonus-firingRate.object") },
				{ "Firing Range", "Affected towers have range of fire increased by 4 tiles.\nEnhancements do not stack.", EnhancementTypeEnum::FiringRange, HashString("mazetd/buildings/bonus-firingRange.object") },
				{ "Splash Radius", "Affected towers have splash radius increased by 2 tiles.\nMana cost is doubled.\nEnhancements do not stack.", EnhancementTypeEnum::SplashRadius, HashString("mazetd/buildings/bonus-splashRadius.object") },
				{ "Intense Utility", "Affected tower's damage over time is applied 5 times faster.\nEnhancements do not stack.", EnhancementTypeEnum::IntenseDot, HashString("mazetd/buildings/bonus-intenseDot.object") },
				{ "Mana Allowance", "Affected towers have third mana cost.\nEnhancements do not stack.", EnhancementTypeEnum::ManaDiscount, HashString("mazetd/buildings/bonus-manaDiscount.object") },
			};

			for (const auto &it : enhancementData)
			{
				Entity *e = generate(1, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.55;
				e->value<CostComponent>().cost = 1000;
				e->value<ModEnhancementComponent>().enhancement = it.enhancement;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			// targeting

			struct TargetingData
			{
				StringPointer name;
				StringPointer description;
				TargetingEnum targeting = TargetingEnum::Random;
				uint32 model = 0;
			};
			static constexpr TargetingData targetingData[] = {
				{ "Front", "Affected towers prefer monsters with shortest remaining path.", TargetingEnum::Front, HashString("mazetd/buildings/targeting-front.object")},
				{ "Back", "Affected towers prefer monsters with longest remaining path.", TargetingEnum::Back, HashString("mazetd/buildings/targeting-back.object") },
				{ "Strongest", "Affected towers prefer monsters with most life.", TargetingEnum::Strongest, HashString("mazetd/buildings/targeting-strongest.object") },
				{ "Weakest", "Affected towers prefer monsters with least life.", TargetingEnum::Weakest, HashString("mazetd/buildings/targeting-weakest.object") },
				{ "Closest", "Affected towers prefer monsters closest to the tower.", TargetingEnum::Closest, HashString("mazetd/buildings/targeting-closest.object") },
				{ "Farthest", "Affected towers prefer monsters farthest from the tower.", TargetingEnum::Farthest, HashString("mazetd/buildings/targeting-farthest.object") },
			};

			for (const auto &it : targetingData)
			{
				Entity *e = generate(2, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.6;
				e->value<CostComponent>().cost = 150;
				e->value<ModTargetingComponent>().targeting = it.targeting;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			// elements

			struct ElementsData
			{
				StringPointer name;
				StringPointer description;
				DamageTypeEnum element = DamageTypeEnum::Total;
				uint32 model = 0;
			};
			static constexpr ElementsData elementsData[] = {
				{ "Fire", "Affected tower's damage is doubled and applied over time.\nThe damage is converted to fire, which hastens monsters movement.\nFire is nullified by water.", DamageTypeEnum::Fire, HashString("mazetd/buildings/augment-fire.object") },
				{ "Water", "Affected tower's damage is doubled and applied over time.\nThe damage is converted to water, which slows down the monsters.\nWater is nullified by fire.", DamageTypeEnum::Water, HashString("mazetd/buildings/augment-water.object") },
				{ "Poison", "Affected tower's damage is doubled and applied over time.\nThe damage poisons the monsters.\nPoisoned monsters take double damage from physical attacks.\nPoison is nullified by magic.", DamageTypeEnum::Poison, HashString("mazetd/buildings/augment-poison.object") },
				{ "Magic", "Affected tower's damage is doubled and applied over time.\nThe damage is converted to magic.\nMonsters affected by magic take double damage from non-physical attacks.\nMagic is dispelled by poison.", DamageTypeEnum::Magic, HashString("mazetd/buildings/augment-magic.object") },
			};

			for (const auto &it : elementsData)
			{
				Entity *e = generate(3, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.4;
				e->value<CostComponent>().cost = 1000;
				e->value<ModElementComponent>().element = it.element;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			// mana

			struct ManaData
			{
				StringPointer name;
				StringPointer description;
				ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
				uint32 amount = 0;
				uint32 model = 0;
			};
			static constexpr ManaData manaData[] = {
				{ "Waterwheel Collector", "Collects mana from surrounding water tiles.", ManaCollectorTypeEnum::Water, 5, HashString("mazetd/buildings/mana-collector-water.object")},
				{ "Sunbloom Collector", "Collects mana from surrounding grass tiles.", ManaCollectorTypeEnum::Sun, 3, HashString("mazetd/buildings/mana-collector-sun.object") },
				{ "Windmill Collector", "Collects mana from surrounding dirt tiles.", ManaCollectorTypeEnum::Wind, 3, HashString("mazetd/buildings/mana-collector-wind.object") },
				{ "Snowmelt Collector", "Collects mana from surrounding snow tiles.", ManaCollectorTypeEnum::Snow, 5, HashString("mazetd/buildings/mana-collector-snow.object") },
			};

			for (const auto &it : manaData)
			{
				Entity *e = generate(4, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1;
				e->value<CostComponent>().cost = 200;
				e->value<ManaStorageComponent>();
				e->value<ManaCollectorComponent>().type = it.type;
				e->value<ManaCollectorComponent>().collectAmount = it.amount;
				e->value<ManaDistributorComponent>().range = e->value<ManaCollectorComponent>().range;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			{
				Entity *e = generate(4, "Mana Relay");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.5;
				e->value<CostComponent>().cost = 100;
				e->value<ManaStorageComponent>();
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>().range = 10;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-relay.object");
				e->value<DescriptionComponent>().description = "Transfers mana over long distances.";
			}

			{
				Entity *e = generate(4, "Mana Capacitor");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.3;
				e->value<CostComponent>().cost = 500;
				e->value<ManaStorageComponent>().capacity = 2000;
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>();
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-capacitor.object");
				e->value<DescriptionComponent>().description = "Stores large amount of mana for later use.";
			}
		}
	};
}

void generateBuildingsList()
{
	BuildingsGenerator generator;
	generator.prepareCategories();
	generator.generateAll();
}

#include <cage-core/hashString.h>
#include <cage-engine/guiComponents.h>
#include <cage-engine/guiManager.h>
#include <cage-simple/engine.h>

#include "../game.h"

void updateSelectedBuildingScreen();

namespace
{
	bool buildingSelectionClick(uint32 i)
	{
		playerBuildingSelection = engineGuiEntities()->get(i);
		updateSelectedBuildingScreen();
		engineGuiManager()->focus(0); // defocus to allow using keyboard shortcuts
		return true;
	}

	struct BuildingsGenerator
	{
		EntityManager *ents = engineGuiEntities();
		uint32 index = 1000;

		static constexpr StringLiteral categoryNames[] = { "Basic", "Towers", "Bonuses", "Targeting", "Elements", "Mana" };

		void prepareCategories()
		{
			sint32 order = 0;
			for (StringLiteral name : categoryNames)
			{
				Entity *e = ents->create(410 + order++);
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 401;
				pp.order = order;
				GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
				GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
				ll.vertical = true;
				GuiTextComponent &txt = e->value<GuiTextComponent>();
				txt.value = String(name);
			}
		}

		Entity *generate(uint32 categoryIndex, StringLiteral name)
		{
			Entity *e = ents->create(++index);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 410 + categoryIndex;
			pp.order = index;
			GuiButtonComponent &but = e->value<GuiButtonComponent>();
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = String(name);
			GuiTextFormatComponent &format = e->value<GuiTextFormatComponent>();
			GuiEventComponent &evt = e->value<GuiEventComponent>();
			evt.event.bind<&buildingSelectionClick>();
			e->value<NameComponent>().name = name;
			return e;
		}

		void generateAll()
		{
			// basic

			{
				Entity *e = generate(0, "Wall");
				e->value<BuildingComponent>();
				e->value<CostComponent>().cost = 4;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/wall.object");
				e->value<DescriptionComponent>().description = "Blocks monsters path, but does not attack.";
			}

			{
				Entity *e = generate(0, "Spikes");
				e->value<TrapComponent>();
				e->value<CostComponent>().cost = 20;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 2;
				d.overTime = 6;
				d.firingPeriod = 6; // 6 shots per second
				d.firingRange = 0.6;
				d.invalidClasses = MonsterClassFlags::Flier;
				d.acceptMods = false;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/trap-spikes.object");
				e->value<DescriptionComponent>().description = "Placed in monsters way, damages monsters walking over it.\nCannot attack flying monsters.\nCannot use any bonuses, targeting, or elements.";
				// dps: 10
				// dps per dollar: 0.5
			}

			// towers

			{
				Entity *e = generate(1, "Mundane");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.1;
				e->value<CostComponent>().cost = 50;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingPeriod = 60; // 0.5 shots per second
				d.damage = 25;
				d.baseManaCost = 25;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-light.object");
				e->value<DescriptionComponent>().description = "Cheapest tower.";
				// dps: 12.5
				// dps per dollar: 0.25
				// damage per mana: 1
				// mana per second: 12.5
			}

			{
				Entity *e = generate(1, "Spiritual");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.8;
				e->value<CostComponent>().cost = 500;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 100;
				d.baseManaCost = 10;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-heavy.object");
				e->value<DescriptionComponent>().description = "Tower with best mana efficiency.";
				// dps: 100
				// dps per dollar: 0.2
				// damage per mana: 10
				// mana per second: 10

				// balance estimation:
				// 20 towers, 7 mana relays, 2 capacitors, 5 collectors, 2 elements, 2 bonuses
				// 12'000 dps (24'000 dps when combining element+magic), 15'000 money, 200 mana per second
			}

			{
				Entity *e = generate(1, "Infused");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.6;
				e->value<CostComponent>().cost = 1000;
				DamageComponent &d = e->value<DamageComponent>();
				d.firingPeriod = 15; // 2 shots per second
				d.damage = 150;
				d.baseManaCost = 50;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-medium.object");
				e->value<DescriptionComponent>().description = "Tower with best DPS per dollar.";
				// dps: 300
				// dps per dollar: 0.3
				// damage per mana: 3
				// mana per second: 100

				// balance estimation:
				// 20 towers, 2 bonuses
				// 12'000 dps (24'000 dps when combining physical+poison), 21'000 money
			}

			// bonuses

			struct BonusData
			{
				StringLiteral name;
				StringLiteral description;
				BonusTypeEnum bonus = BonusTypeEnum::None;
				uint32 model = 0;
			};
			static constexpr BonusData bonusData[] = {
				{ "Damage", "Affected towers have doubled damage and mana cost.", BonusTypeEnum::Damage, HashString("mazetd/buildings/bonus-damage.object")},
				{ "Firing Rate", "Affected towers have doubled rate of fire.", BonusTypeEnum::FiringRate, HashString("mazetd/buildings/bonus-firingRate.object") },
				{ "Firing Range", "Affected towers have range of fire increased by 4 tiles.", BonusTypeEnum::FiringRange, HashString("mazetd/buildings/bonus-firingRange.object") },
				{ "Splash Radius", "Affected towers have splash radius increased by 2 tiles. Mana cost is tripled.", BonusTypeEnum::SplashRadius, HashString("mazetd/buildings/bonus-splashRadius.object") },
				{ "Intense Utility", "Affected tower's damage over time is applied 5 times faster.", BonusTypeEnum::IntenseDot, HashString("mazetd/buildings/bonus-intenseDot.object") },
				{ "Mana Allowance", "Affected towers have one third mana cost.", BonusTypeEnum::ManaDiscount, HashString("mazetd/buildings/bonus-manaDiscount.object") },
			};

			for (const auto &it : bonusData)
			{
				Entity *e = generate(2, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.55;
				e->value<CostComponent>().cost = 500;
				e->value<ModBonusComponent>().type = it.bonus;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			// targeting

			struct TargetingData
			{
				StringLiteral name;
				StringLiteral description;
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
				Entity *e = generate(3, it.name);
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
				StringLiteral name;
				StringLiteral description;
				DamageTypeEnum element = DamageTypeEnum::Total;
				uint32 model = 0;
			};
			static constexpr ElementsData elementsData[] = {
				{ "Fire", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to fire, which hastens monsters movement.\nFire is nullified by water.\nThe towers require mana to be able to fire.", DamageTypeEnum::Fire, HashString("mazetd/buildings/augment-fire.object") },
				{ "Water", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to water, which slows down the monsters.\nWater is nullified by fire.\nThe towers require mana to be able to fire.", DamageTypeEnum::Water, HashString("mazetd/buildings/augment-water.object") },
				{ "Poison", "Affected tower's damage is tripled and applied over time.\nThe damage poisons the monsters.\nPoisoned monsters take double damage from physical attacks.\nPoison is nullified by magic.\nThe towers require mana to be able to fire.", DamageTypeEnum::Poison, HashString("mazetd/buildings/augment-poison.object") },
				{ "Magic", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to magic.\nMonsters affected by magic take double damage from non-physical attacks.\nMagic is dispelled by poison.\nThe towers require mana to be able to fire.", DamageTypeEnum::Magic, HashString("mazetd/buildings/augment-magic.object") },
			};

			for (const auto &it : elementsData)
			{
				Entity *e = generate(4, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.4;
				e->value<CostComponent>().cost = 400;
				e->value<ModElementComponent>().element = it.element;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			// mana

			struct ManaData
			{
				StringLiteral name;
				StringLiteral description;
				ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
				uint32 amount = 0;
				uint32 model = 0;
			};
			static constexpr ManaData manaData[] = {
				{ "Waterwheel Collector", "Collects mana from surrounding water tiles.", ManaCollectorTypeEnum::Water, 10, HashString("mazetd/buildings/mana-collector-water.object")},
				{ "Sunbloom Collector", "Collects mana from surrounding grass tiles.", ManaCollectorTypeEnum::Sun, 5, HashString("mazetd/buildings/mana-collector-sun.object") },
				{ "Windmill Collector", "Collects mana from surrounding dirt tiles.", ManaCollectorTypeEnum::Wind, 5, HashString("mazetd/buildings/mana-collector-wind.object") },
				{ "Snowmelt Collector", "Collects mana from surrounding snow tiles.", ManaCollectorTypeEnum::Snow, 10, HashString("mazetd/buildings/mana-collector-snow.object") },
			};

			for (const auto &it : manaData)
			{
				Entity *e = generate(5, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1;
				e->value<CostComponent>().cost = 300;
				e->value<ManaStorageComponent>();
				e->value<ManaDistributorComponent>();
				e->value<ManaCollectorComponent>().type = it.type;
				e->value<ManaCollectorComponent>().collectAmount = it.amount;
				e->value<GuiModelComponent>().model = it.model;
				e->value<DescriptionComponent>().description = it.description;
			}

			{
				Entity *e = generate(5, "Mana Relay");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.5;
				e->value<CostComponent>().cost = 100;
				e->value<ManaStorageComponent>();
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>().range = 10;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-relay.object");
				e->value<DescriptionComponent>().description = "Transfers mana over longer distances.";
			}

			{
				Entity *e = generate(5, "Mana Capacitor");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.3;
				e->value<CostComponent>().cost = 500;
				e->value<ManaStorageComponent>().capacity = 1500;
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>();
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-capacitor.object");
				e->value<DescriptionComponent>().description = "Stores larger amount of mana for later use.";
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

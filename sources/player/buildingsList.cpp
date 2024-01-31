#include "../game.h"

#include <cage-core/hashString.h>
#include <cage-engine/guiBuilder.h>
#include <cage-engine/guiManager.h>
#include <cage-simple/engine.h>

namespace mazetd
{
	namespace
	{
		bool buildingSelectionClick(input::GuiValue in)
		{
			playerBuildingSelection = in.entity;
			engineGuiManager()->focus(0); // defocus to allow using keyboard shortcuts
			return true;
		}

		void buildingTooltip(const GuiTooltipConfig &config)
		{
			Entity *sb = config.invoker;
			if (!sb)
				return;

			Holder<GuiBuilder> g = newGuiBuilder(config.tooltip);
			auto _1 = g->panel().text(Stringizer() + sb->value<NameComponent>().name);
			auto _2 = g->column();
			g->label().text(Stringizer() + sb->value<DescriptionComponent>().description);

			// damage
			if (sb->has<DamageComponent>())
			{
				g->label().text(Stringizer() + "Base damage: " + sb->value<DamageComponent>().damage);
				g->label().text(Stringizer() + "Base damage per second: " + (30.f * sb->value<DamageComponent>().damage / sb->value<DamageComponent>().firingPeriod));
				g->label().text(Stringizer() + "Mana cost: " + sb->value<DamageComponent>().manaCost);
			}

			// mana collector
			if (sb->has<ManaCollectorComponent>())
				g->label().text(Stringizer() + "Harvested mana multiplier: " + sb->value<ManaCollectorComponent>().collectAmount);

			// mana storage
			if (sb->has<ManaStorageComponent>())
				g->label().text(Stringizer() + "Mana capacity: " + sb->value<ManaStorageComponent>().capacity);

			{ // cost
				CAGE_ASSERT(sb->has<CostComponent>());
				g->label().text(Stringizer() + "Cost: " + sb->value<CostComponent>().cost);
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
				e->value<GuiEventComponent>().event = inputFilter(buildingSelectionClick);
				e->value<NameComponent>().name = name;
				e->value<GuiTooltipComponent>().tooltip.bind<buildingTooltip>();
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
					e->value<CostComponent>().cost = 4;
					e->value<GuiModelComponent>().model = HashString("mazetd/buildings/wall.object");
					e->value<DescriptionComponent>().description = "Blocks monsters path, but does not attack.";
				}

				// damage

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

				{
					Entity *e = generate(0, "Mundane");
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 2.1;
					e->value<CostComponent>().cost = 50;
					DamageComponent &d = e->value<DamageComponent>();
					d.firingPeriod = 60; // 0.5 shots per second
					d.damage = 25;
					d.manaCost = 25;
					e->value<ManaStorageComponent>().capacity = 100;
					e->value<ManaReceiverComponent>();
					e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-light.object");
					e->value<DescriptionComponent>().description = "Cheapest tower.";
					// dps: 12.5
					// dps per dollar: 0.25
					// damage per mana: 1
					// mana per second: 12.5
				}

				{
					Entity *e = generate(0, "Spiritual");
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 2.8;
					e->value<CostComponent>().cost = 500;
					DamageComponent &d = e->value<DamageComponent>();
					d.damage = 100;
					d.manaCost = 10;
					e->value<ManaStorageComponent>().capacity = 100;
					e->value<ManaReceiverComponent>();
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
					Entity *e = generate(0, "Reinforced");
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 2.6;
					e->value<CostComponent>().cost = 1000;
					DamageComponent &d = e->value<DamageComponent>();
					d.firingPeriod = 15; // 2 shots per second
					d.damage = 150;
					d.manaCost = 50;
					e->value<ManaStorageComponent>().capacity = 200;
					e->value<ManaReceiverComponent>();
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

				// enhancements

				struct BonusData
				{
					StringPointer name;
					StringPointer description;
					BonusTypeEnum bonus = BonusTypeEnum::None;
					uint32 model = 0;
				};
				static constexpr BonusData bonusData[] = {
					{ "Damage", "Affected towers have doubled damage and mana cost.\nEnhancements cannot be combined.", BonusTypeEnum::Damage, HashString("mazetd/buildings/bonus-damage.object") },
					{ "Rate", "Affected towers have doubled rate of fire.\nEnhancements cannot be combined.", BonusTypeEnum::FiringRate, HashString("mazetd/buildings/bonus-firingRate.object") },
					{ "Range", "Affected towers have range of fire increased by 4 tiles.\nEnhancements cannot be combined.", BonusTypeEnum::FiringRange, HashString("mazetd/buildings/bonus-firingRange.object") },
					{ "Splash", "Affected towers have splash radius increased by 2 tiles. Mana cost is tripled.\nEnhancements cannot be combined.", BonusTypeEnum::SplashRadius, HashString("mazetd/buildings/bonus-splashRadius.object") },
					{ "Intense", "Affected tower's damage over time is applied 5 times faster.\nEnhancements cannot be combined.", BonusTypeEnum::IntenseDot, HashString("mazetd/buildings/bonus-intenseDot.object") },
					{ "Mana", "Affected towers have one third mana cost.\nEnhancements cannot be combined.", BonusTypeEnum::ManaDiscount, HashString("mazetd/buildings/bonus-manaDiscount.object") },
				};

				for (const auto &it : bonusData)
				{
					Entity *e = generate(1, it.name);
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 1.55;
					e->value<CostComponent>().cost = 500;
					e->value<ModBonusComponent>().bonus = it.bonus;
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
					{ "Front", "Affected towers prefer monsters with shortest remaining path.", TargetingEnum::Front, HashString("mazetd/buildings/targeting-front.object") },
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
					{ "Fire", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to fire, which hastens monsters movement.\nFire is nullified by water.", DamageTypeEnum::Fire, HashString("mazetd/buildings/augment-fire.object") },
					{ "Water", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to water, which slows down the monsters.\nWater is nullified by fire.", DamageTypeEnum::Water, HashString("mazetd/buildings/augment-water.object") },
					{ "Poison", "Affected tower's damage is tripled and applied over time.\nThe damage poisons the monsters.\nPoisoned monsters take double damage from physical attacks.\nPoison is nullified by magic.", DamageTypeEnum::Poison, HashString("mazetd/buildings/augment-poison.object") },
					{ "Magic", "Affected tower's damage is tripled and applied over time.\nThe damage is converted to magic.\nMonsters affected by magic take double damage from non-physical attacks.\nMagic is dispelled by poison.", DamageTypeEnum::Magic, HashString("mazetd/buildings/augment-magic.object") },
				};

				for (const auto &it : elementsData)
				{
					Entity *e = generate(3, it.name);
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
					StringPointer name;
					StringPointer description;
					ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
					uint32 amount = 0;
					uint32 model = 0;
				};
				static constexpr ManaData manaData[] = {
					{ "Waterwheel", "Collects mana from surrounding water tiles.", ManaCollectorTypeEnum::Water, 10, HashString("mazetd/buildings/mana-collector-water.object") },
					{ "Sunbloom", "Collects mana from surrounding grass tiles.", ManaCollectorTypeEnum::Sun, 5, HashString("mazetd/buildings/mana-collector-sun.object") },
					{ "Windmill", "Collects mana from surrounding dirt tiles.", ManaCollectorTypeEnum::Wind, 5, HashString("mazetd/buildings/mana-collector-wind.object") },
					{ "Snowmelt", "Collects mana from surrounding snow tiles.", ManaCollectorTypeEnum::Snow, 10, HashString("mazetd/buildings/mana-collector-snow.object") },
				};

				for (const auto &it : manaData)
				{
					Entity *e = generate(4, it.name);
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 1;
					e->value<CostComponent>().cost = 300;
					e->value<ManaStorageComponent>().capacity = 100;
					e->value<ManaDistributorComponent>();
					e->value<ManaCollectorComponent>().type = it.type;
					e->value<ManaCollectorComponent>().collectAmount = it.amount;
					e->value<GuiModelComponent>().model = it.model;
					e->value<DescriptionComponent>().description = it.description;
				}

				{
					Entity *e = generate(4, "Relay");
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 2.5;
					e->value<CostComponent>().cost = 100;
					e->value<ManaStorageComponent>().capacity = 100;
					e->value<ManaStorageComponent>().transferOrdering = -1;
					e->value<ManaReceiverComponent>();
					e->value<ManaDistributorComponent>();
					e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-relay.object");
					e->value<DescriptionComponent>().description = "Directs mana transfers.";
				}

				{
					Entity *e = generate(4, "Capacitor");
					e->value<BuildingComponent>();
					e->value<PivotComponent>().elevation = 1.3;
					e->value<CostComponent>().cost = 500;
					e->value<ManaStorageComponent>().capacity = 1500;
					e->value<ManaStorageComponent>().transferOrdering = 1;
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
}

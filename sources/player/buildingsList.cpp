#include <cage-core/hashString.h>
#include <cage-engine/engine.h>
#include <cage-engine/gui.h>

#include "../game.h"

namespace
{
	bool buildingSelectionClick(uint32 i)
	{
		playerBuildingSelection = engineGui()->entities()->get(i);
		engineGui()->focus(0); // defocus to allow using keyboard shortcuts
		return true;
	}

	struct BuildingsGenerator
	{
		EntityManager *ents = engineGui()->entities();
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
			e->value<NameComponent>().name = String(name);
			return e;
		}

		void generateAll()
		{
			// basic

			{
				Entity *e = generate(0, "Wall");
				e->value<BuildingComponent>();
				e->value<CostComponent>().cost = 5;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/wall.object");
			}

			{
				Entity *e = generate(0, "Spikes");
				e->value<TrapComponent>();
				e->value<CostComponent>().cost = 30;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 3;
				d.overTime = 6;
				d.firingPeriod = 6;
				d.firingRange = 0.5;
				d.splashRadius = 0.25;
				d.invalidClasses = MonsterClassFlags::Flyer;
				d.acceptMods = false;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/trap-spikes.object");
			}

			// towers

			{
				Entity *e = generate(1, "Light");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.1;
				e->value<CostComponent>().cost = 100;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 10;
				d.baseManaCost = 12;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-light.object");
			}

			{
				Entity *e = generate(1, "Medium");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.6;
				e->value<CostComponent>().cost = 2000;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 100;
				d.baseManaCost = 100;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-medium.object");
			}

			{
				Entity *e = generate(1, "Heavy");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.8;
				e->value<CostComponent>().cost = 40000;
				DamageComponent &d = e->value<DamageComponent>();
				d.damage = 1000;
				d.baseManaCost = 800;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/tower-heavy.object");
			}

			// bonuses

			struct BonusData
			{
				StringLiteral name;
				BonusTypeEnum bonus = BonusTypeEnum::None;
				uint32 model = 0;
			};
			constexpr BonusData bonusData[] = {
				{ "Damage", BonusTypeEnum::Damage, HashString("mazetd/buildings/bonus-damage.object") },
				{ "Firing Rate", BonusTypeEnum::FiringRate, HashString("mazetd/buildings/bonus-firingRate.object") },
				{ "Firing Range", BonusTypeEnum::FiringRange, HashString("mazetd/buildings/bonus-firingRange.object") },
				{ "Splash Radius", BonusTypeEnum::SplashRadius, HashString("mazetd/buildings/bonus-splashRadius.object") },
				{ "Intense Dot", BonusTypeEnum::IntenseDot, HashString("mazetd/buildings/bonus-intenseDot.object") },
				{ "Mana Discount", BonusTypeEnum::ManaDiscount, HashString("mazetd/buildings/bonus-manaDiscount.object") },
			};

			for (const auto &it : bonusData)
			{
				Entity *e = generate(2, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.55;
				e->value<CostComponent>().cost = 300;
				e->value<ModBonusComponent>().type = it.bonus;
				e->value<GuiModelComponent>().model = it.model;
			}

			// targeting

			struct TargetingData
			{
				StringLiteral name;
				TargetingEnum targeting = TargetingEnum::Random;
				uint32 model = 0;
			};
			constexpr TargetingData targetingData[] = {
				{ "Front", TargetingEnum::Front, HashString("mazetd/buildings/targeting-front.object") },
				{ "Back", TargetingEnum::Back, HashString("mazetd/buildings/targeting-back.object") },
				{ "Strongest", TargetingEnum::Strongest, HashString("mazetd/buildings/targeting-strongest.object") },
				{ "Weakest", TargetingEnum::Weakest, HashString("mazetd/buildings/targeting-weakest.object") },
				{ "Closest", TargetingEnum::Closest, HashString("mazetd/buildings/targeting-closest.object") },
				{ "Farthest", TargetingEnum::Farthest, HashString("mazetd/buildings/targeting-farthest.object") },
			};

			for (const auto &it : targetingData)
			{
				Entity *e = generate(3, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.6;
				e->value<CostComponent>().cost = 250;
				e->value<ModTargetingComponent>().targeting = it.targeting;
				e->value<GuiModelComponent>().model = it.model;
			}

			// elements

			struct ElementsData
			{
				StringLiteral name;
				DamageTypeEnum element = DamageTypeEnum::Total;
				uint32 model = 0;
			};
			constexpr ElementsData elementsData[] = {
				{ "Fire", DamageTypeEnum::Fire, HashString("mazetd/buildings/augment-fire.object") },
				{ "Water", DamageTypeEnum::Water, HashString("mazetd/buildings/augment-water.object") },
				{ "Poison", DamageTypeEnum::Poison, HashString("mazetd/buildings/augment-poison.object") },
				{ "Magic", DamageTypeEnum::Magic, HashString("mazetd/buildings/augment-magic.object") },
			};

			for (const auto &it : elementsData)
			{
				Entity *e = generate(4, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.4;
				e->value<CostComponent>().cost = 500;
				e->value<ModElementComponent>().element = it.element;
				e->value<GuiModelComponent>().model = it.model;
			}

			// mana

			struct ManaData
			{
				StringLiteral name;
				ManaCollectorTypeEnum type = ManaCollectorTypeEnum::None;
				uint32 amount = 0;
				uint32 model = 0;
			};
			constexpr ManaData manaData[] = {
				{ "Waterwheel Collector", ManaCollectorTypeEnum::Water, 10, HashString("mazetd/buildings/mana-collector-water.object") },
				{ "Sunbloom Collector", ManaCollectorTypeEnum::Sun, 5, HashString("mazetd/buildings/mana-collector-sun.object") },
				{ "Windmill Collector", ManaCollectorTypeEnum::Wind, 5, HashString("mazetd/buildings/mana-collector-wind.object") },
				{ "Snowmelt Collector", ManaCollectorTypeEnum::Snow, 10, HashString("mazetd/buildings/mana-collector-snow.object") },
			};

			for (const auto &it : manaData)
			{
				Entity *e = generate(5, it.name);
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1;
				e->value<CostComponent>().cost = 500;
				e->value<ManaStorageComponent>();
				e->value<ManaDistributorComponent>();
				e->value<ManaCollectorComponent>().type = it.type;
				e->value<ManaCollectorComponent>().collectAmount = it.amount;
				e->value<GuiModelComponent>().model = it.model;
			}

			{
				Entity *e = generate(5, "Mana Relay");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 2.5;
				e->value<CostComponent>().cost = 200;
				e->value<ManaStorageComponent>();
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>().range = 10;
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-relay.object");
			}

			{
				Entity *e = generate(5, "Mana Capacitor");
				e->value<BuildingComponent>();
				e->value<PivotComponent>().elevation = 1.3;
				e->value<CostComponent>().cost = 3000;
				e->value<ManaStorageComponent>().capacity = 5000;
				e->value<ManaReceiverComponent>();
				e->value<ManaDistributorComponent>();
				e->value<GuiModelComponent>().model = HashString("mazetd/buildings/mana-capacitor.object");
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

#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>

#include "screens.h"
#include "../grid.h"
#include "../game.h"

void setScreenGameMenu();
void generateBuildingsList();

namespace
{
	EventListener<void()> engineUpdateListener;
	EventListener<void()> guiCleanListener;

	bool buttonMenu(uint32)
	{
		setScreenGameMenu();
		return true;
	}

	void updateTopBar()
	{
		EntityManager *ents = engineGuiEntities();

		{ // path tiles
			Entity *e = ents->get(312);
			e->value<GuiTextComponent>().value = Stringizer() + globalWaypoints->minFullDistance;
		}

		{ // health
			Entity *e = ents->get(314);
			e->value<GuiTextComponent>().value = Stringizer() + playerHealth;
		}

		{ // dollars
			Entity *e = ents->get(316);
			e->value<GuiTextComponent>().value = Stringizer() + playerMoney;
		}

		{ // mana
			sint64 manaCap = 1;
			sint64 manaAvail = 0;
			entitiesVisitor([&](const ManaStorageComponent &mc) {
				manaCap += mc.capacity;
				manaAvail += mc.mana;
				}, gameEntities(), false);
			Entity *e = ents->get(318);
			e->value<GuiTextComponent>().value = Stringizer() + (100 * manaAvail / manaCap) + " %";
		}

		{ // monsters
			uint32 cnt = 0;
			sint64 lfTot = 1;
			sint64 lfRem = 0;
			entitiesVisitor([&](const MonsterComponent &mc) {
				cnt++;
				lfTot += mc.maxLife;
				lfRem += mc.life;
			}, gameEntities(), false);
			Entity *e = ents->get(320);
			e->value<GuiTextComponent>().value = Stringizer() + cnt + " @ " + (100 * lfRem / lfTot) + " %";
		}
	}

	void engineUpdate()
	{
		updateTopBar();
		engineGuiEntities()->get(521)->value<GuiTextComponent>().value = gamePaused ? "Paused" : "";
	}

	void guiClean()
	{
		engineUpdateListener.detach();
		gameReady = false;
	}
}

void updateSelectedBuildingScreen()
{
	removeGuiEntitiesWithParent(601);

	Entity *sb = playerBuildingSelection;
	if (!sb)
		return;

	EntityManager *ents = engineGuiEntities();
	sint32 index = 0;

	{ // name
		CAGE_ASSERT(sb->has<NameComponent>());
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 601;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + sb->value<NameComponent>().name;
	}

	{ // description
		CAGE_ASSERT(sb->has<DescriptionComponent>());
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 601;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = String(sb->value<DescriptionComponent>().description);
	}

	{ // cost
		CAGE_ASSERT(sb->has<CostComponent>());
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 601;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Cost: " + sb->value<CostComponent>().cost;
	}

	// damage
	if (sb->has<DamageComponent>())
	{
		{ // damage
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 601;
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Base damage: " + sb->value<DamageComponent>().damage;
		}
		{ // dps
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 601;
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Base damage per second: " + (30.f * sb->value<DamageComponent>().damage / sb->value<DamageComponent>().firingPeriod);
		}
		{ // baseManaCost
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 601;
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Base mana cost: " + sb->value<DamageComponent>().baseManaCost;
		}
	}

	// mana collector
	if (sb->has<ManaCollectorComponent>())
	{
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 601;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Harvested mana multiplier: " + sb->value<ManaCollectorComponent>().collectAmount;
	}

	// mana storage
	if (sb->has<ManaStorageComponent>())
	{
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 601;
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
			pp.parent = 601;
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Mana transfer rate: " + sb->value<ManaDistributorComponent>().transferLimit;
		}
		{ // transfer range
			Entity *e = ents->createUnique();
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 601;
			pp.order = index++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = Stringizer() + "Mana transfer range: " + sb->value<ManaDistributorComponent>().range;
		}
	}
}

void updateSpawningMonsterPropertiesScreen()
{
	removeGuiEntitiesWithParent(501);

	const SpawningGroup &mo = spawningGroup;
	if (!mo.name)
		return;

	EntityManager *ents = engineGuiEntities();
	sint32 index = 0;

	{ // name
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + SpawningGroup::waveIndex + ": " + mo.name;
	}

	if (mo.monsterClass != MonsterClassFlags::None)
	{ // monster class
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Class:";
		struct Pair
		{
			MonsterClassFlags flag = MonsterClassFlags::None;
			StringLiteral name;
		};
		constexpr const Pair pairs[] = {
			Pair{ MonsterClassFlags::Flier, "Flier" },
			Pair{ MonsterClassFlags::Boss, "Boss" },
		};
		for (const auto &it : pairs)
			if (any(mo.monsterClass & it.flag))
				txt.value += Stringizer() + " " + String(it.name);
	}

	if (mo.resistances != DamageTypeFlags::None)
	{ // resistances
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Resistances:";
		struct Pair
		{
			DamageTypeFlags flag = DamageTypeFlags::None;
			StringLiteral name;
		};
		constexpr const Pair pairs[] = {
			Pair{ DamageTypeFlags::Physical, "Physical" },
			Pair{ DamageTypeFlags::Fire, "Fire" },
			Pair{ DamageTypeFlags::Water, "Water" },
			Pair{ DamageTypeFlags::Poison, "Poison" },
			Pair{ DamageTypeFlags::Magic, "Magic" },
		};
		for (const auto &it : pairs)
			if (any(mo.resistances & it.flag))
				txt.value += Stringizer() + " " + String(it.name);
	}

	{ // count
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Count: " + (mo.spawnCount * mo.spawnSimultaneously);
	}

	{ // life
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Life: " + mo.maxLife;
	}

	{ // speed
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Speed: " + mo.speed;
	}

	{ // money
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = Stringizer() + "Reward: " + mo.money;
	}
}

void setScreenGame()
{
	cleanGui();
	EntityManager *ents = engineGuiEntities();
	guiCleanListener.attach(eventGuiClean());
	guiCleanListener.bind<&guiClean>();
	engineUpdateListener.attach(controlThread().update);
	engineUpdateListener.bind<&engineUpdate>();
	gameReady = true;

	// top bar

	{
		Entity *e = ents->create(300);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.5, 0);
	}

	{
		Entity *e = ents->create(301);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 300;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(310);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 1;
		GuiButtonComponent &but = e->value<GuiButtonComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/menu.png");
		GuiEventComponent &ev = e->value<GuiEventComponent>();
		ev.event.bind<&buttonMenu>();
	}

	{
		Entity *e = ents->create(302);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 2;
		GuiPanelComponent &pnl = e->value<GuiPanelComponent>();
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(311);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 302;
		pp.order = 1;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/path.png");
	}

	{
		Entity *e = ents->create(312);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 302;
		pp.order = 2;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "path";
	}

	{
		Entity *e = ents->create(303);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 3;
		GuiPanelComponent &pnl = e->value<GuiPanelComponent>();
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(313);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 303;
		pp.order = 1;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/health.png");
	}

	{
		Entity *e = ents->create(314);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 303;
		pp.order = 2;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "health";
	}

	{
		Entity *e = ents->create(304);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 4;
		GuiPanelComponent &pnl = e->value<GuiPanelComponent>();
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(315);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 304;
		pp.order = 1;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/dollar.png");
	}

	{
		Entity *e = ents->create(316);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 304;
		pp.order = 2;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "dollars";
	}

	{
		Entity *e = ents->create(305);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 5;
		GuiPanelComponent &pnl = e->value<GuiPanelComponent>();
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(317);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 305;
		pp.order = 1;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/mana.png");
	}

	{
		Entity *e = ents->create(318);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 305;
		pp.order = 2;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "mana";
	}

	{
		Entity *e = ents->create(306);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 301;
		pp.order = 6;
		GuiPanelComponent &pnl = e->value<GuiPanelComponent>();
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(319);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 306;
		pp.order = 1;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		GuiImageComponent &img = e->value<GuiImageComponent>();
		img.textureName = HashString("mazetd/gui/monster.png");
	}

	{
		Entity *e = ents->create(320);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 306;
		pp.order = 2;
		GuiLabelComponent &lb = e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "monsters";
	}

	// buildings menu

	{
		Entity *e = ents->create(400);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(1, 0);
	}

	{
		Entity *e = ents->create(401);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 400;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
	}

	generateBuildingsList();

	// building properties

	{
		Entity *e = ents->create(600);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(1, 1);
	}

	{
		Entity *e = ents->create(601);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 600;
		GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
		sp.collapsed = false;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
		e->value<GuiTextComponent>().value = "Building";
	}

	updateSelectedBuildingScreen();

	// monster properties

	{
		Entity *e = ents->create(500);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0, 0);
	}

	{
		Entity *e = ents->create(501);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 500;
		GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
		sp.collapsed = false;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
		e->value<GuiTextComponent>().value = "Spawning";
	}

	updateSpawningMonsterPropertiesScreen();

	// bottom status bar

	{
		Entity *e = ents->create(520);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = Vec2(0.5, 0.99);
	}

	{
		Entity *e = ents->create(521);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 520;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "Paused";
	}
}

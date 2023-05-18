#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>

#include "screens.h"
#include "../grid.h"
#include "../game.h"

void setScreenPaused();
void generateBuildingsList();

namespace
{
	EventListener<void()> engineUpdateListener;
	EventListener<void()> guiCleanListener;

	bool buttonMenu(Entity *)
	{
		setScreenPaused();
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

	void removeGuiEntitiesWithParent(uint32 parent)
	{
		std::vector<uint32> ents;
		entitiesVisitor([&](Entity *e, const GuiParentComponent &p) {
			if (p.parent == parent)
			{
				ents.push_back(e->name());
				e->destroy();
			}
			}, engineGuiEntities(), true);
		for (uint32 n : ents)
			removeGuiEntitiesWithParent(n);
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
			StringPointer name;
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
			StringPointer name;
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
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0.5, 0);
	}

	{
		Entity *e = ents->create(301);
		e->value<GuiParentComponent>().parent = 300;
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(310);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiButtonComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/menu.png");
		e->value<GuiEventComponent>().event.bind<&buttonMenu>();
	}

	{
		Entity *e = ents->create(302);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiPanelComponent>();
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(311);
		e->value<GuiParentComponent>().parent = 302;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/path.png");
	}

	{
		Entity *e = ents->create(312);
		e->value<GuiParentComponent>().parent = 302;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "path";
	}

	{
		Entity *e = ents->create(303);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 3;
		e->value<GuiPanelComponent>();
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(313);
		e->value<GuiParentComponent>().parent = 303;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/health.png");
	}

	{
		Entity *e = ents->create(314);
		e->value<GuiParentComponent>().parent = 303;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "health";
	}

	{
		Entity *e = ents->create(304);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 4;
		e->value<GuiPanelComponent>();
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(315);
		e->value<GuiParentComponent>().parent = 304;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/dollar.png");
	}

	{
		Entity *e = ents->create(316);
		e->value<GuiParentComponent>().parent = 304;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "dollars";
	}

	{
		Entity *e = ents->create(305);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 5;
		e->value<GuiPanelComponent>();
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(317);
		e->value<GuiParentComponent>().parent = 305;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/mana.png");
	}

	{
		Entity *e = ents->create(318);
		e->value<GuiParentComponent>().parent = 305;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "mana";
	}

	{
		Entity *e = ents->create(306);
		e->value<GuiParentComponent>().parent = 301;
		e->value<GuiParentComponent>().order = 6;
		e->value<GuiPanelComponent>();
		e->value<GuiLayoutLineComponent>();
	}

	{
		Entity *e = ents->create(319);
		e->value<GuiParentComponent>().parent = 306;
		e->value<GuiParentComponent>().order = 1;
		e->value<GuiLabelComponent>();
		e->value<GuiImageComponent>().textureName = HashString("mazetd/gui/monster.png");
	}

	{
		Entity *e = ents->create(320);
		e->value<GuiParentComponent>().parent = 306;
		e->value<GuiParentComponent>().order = 2;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "monsters";
	}

	// buildings menu

	{
		Entity *e = ents->create(400);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(1, 0.5);
	}

	{
		Entity *e = ents->create(401);
		e->value<GuiParentComponent>().parent = 400;
		e->value<GuiLayoutLineComponent>().vertical = true;
	}

	generateBuildingsList();

	// monster properties

	{
		Entity *e = ents->create(500);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0, 0);
	}

	{
		Entity *e = ents->create(501);
		e->value<GuiParentComponent>().parent = 500;
		e->value<GuiSpoilerComponent>().collapsed = false;
		e->value<GuiLayoutLineComponent>().vertical = true;
		e->value<GuiTextComponent>().value = "Spawning";
		e->value<GuiWidgetStateComponent>().skinIndex = 2; // compact skin
	}

	updateSpawningMonsterPropertiesScreen();

	// bottom status bar

	{
		Entity *e = ents->create(520);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0.5, 0.99);
	}

	{
		Entity *e = ents->create(521);
		e->value<GuiParentComponent>().parent = 520;
		e->value<GuiLabelComponent>();
		e->value<GuiTextComponent>().value = "Paused";
	}

	// controls

	{
		Entity *e = ents->create(800);
		e->value<GuiLayoutAlignmentComponent>().alignment = Vec2(0, 1);
	}

	{
		Entity *e = ents->create(801);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 800;
		e->value<GuiSpoilerComponent>().collapsed = false;
		e->value<GuiLayoutLineComponent>().vertical = true;
		e->value<GuiTextComponent>().value = "Controls";
		e->value<GuiWidgetStateComponent>().skinIndex = 2; // compact skin
	}

	{
		static constexpr const char *lines[] = {
			"LMB - place selected building",
			"MMB - destroy building",
			"RMB - move camera",
			"Wheel - zoom",
			"WSAD - move camera",
			"QE - rotate camera",
			"C - change camera mode",
			"Spacebar - pause the game",
			"PGUP/DN - speed up/down the game",
			"Home - reset game speed",
		};

		uint32 idx = 0;
		for (const auto &it : lines)
		{
			Entity *e = engineGuiEntities()->createUnique();
			e->value<GuiParentComponent>().parent = 801;
			e->value<GuiParentComponent>().order = idx++;
			e->value<GuiLabelComponent>();
			e->value<GuiTextComponent>().value = it;
		}
	}
}

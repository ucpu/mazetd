#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>

#include "screens.h"
#include "../grid.h"
#include "../game.h"

uint32 structureMoneyCost(uint32 id);

void setScreenGameMenu();

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
		EntityManager *ents = engineGui()->entities();

		{ // path tiles
			Entity *e = ents->get(312);
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = stringizer() + globalWaypoints->minFullDistance;
		}

		{ // health
			Entity *e = ents->get(314);
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = stringizer() + playerHealth;
		}

		{ // dollars
			Entity *e = ents->get(316);
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = stringizer() + playerMoney;
		}
	}

	void updateCursor()
	{
		removeGuiEntitiesWithParent(201);

		if (playerCursorTile == m)
			return;

		EntityManager *ents = engineGui()->entities();
		sint32 index = 0;

		entitiesVisitor([&](Entity *g, const PositionComponent &po, const NameComponent &nm) {
			if (po.tile != playerCursorTile)
				return;

			{ // name
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 201;
				pp.order = index++;
				GuiLabelComponent &lab = e->value<GuiLabelComponent>();
				GuiTextComponent &txt = e->value<GuiTextComponent>();
				txt.value = nm.name;
			}

			if (g->has<ManaStorageComponent>())
			{ // mana
				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 201;
				pp.order = index++;
				GuiLabelComponent &lab = e->value<GuiLabelComponent>();
				GuiTextComponent &txt = e->value<GuiTextComponent>();
				txt.value = stringizer() + "Mana: " + g->value<ManaStorageComponent>().mana + " / " + g->value<ManaStorageComponent>().capacity;
			}

			if (g->has<MonsterComponent>())
			{ // monster
				{ // life
					Entity *e = ents->createUnique();
					GuiParentComponent &pp = e->value<GuiParentComponent>();
					pp.parent = 201;
					pp.order = index++;
					GuiLabelComponent &lab = e->value<GuiLabelComponent>();
					GuiTextComponent &txt = e->value<GuiTextComponent>();
					txt.value = stringizer() + "Life: " + g->value<MonsterComponent>().life;
				}
				{ // waypoints
					Entity *e = ents->createUnique();
					GuiParentComponent &pp = e->value<GuiParentComponent>();
					pp.parent = 201;
					pp.order = index++;
					GuiLabelComponent &lab = e->value<GuiLabelComponent>();
					GuiTextComponent &txt = e->value<GuiTextComponent>();
					txt.value = stringizer() + "Waypoints: " + bitCount(g->value<MonsterComponent>().visitedWaypointsBits);
				}
			}
		}, gameEntities(), false);

		{
			struct Pair
			{
				TileFlags flag = TileFlags::None;
				StringLiteral name;
			};

			constexpr const Pair pairs[] = {
				Pair{ TileFlags::Mana, "Harvestable Mana" },
				Pair{ TileFlags::Waypoint, "Monsters Waypoint" },
				Pair{ TileFlags::Water, "Water Tile" },
				Pair{ TileFlags::Sun, "Sun Tile" },
				Pair{ TileFlags::Wind, "Wind Tile" },
				Pair{ TileFlags::Snow, "Snow Tile" },
			};

			const TileFlags flags = globalGrid->flags[playerCursorTile];

			for (const auto &p : pairs)
			{
				if (none(flags & p.flag))
					continue;

				Entity *e = ents->createUnique();
				GuiParentComponent &pp = e->value<GuiParentComponent>();
				pp.parent = 201;
				pp.order = index++;
				GuiLabelComponent &lab = e->value<GuiLabelComponent>();
				GuiTextComponent &txt = e->value<GuiTextComponent>();
				txt.value = string(p.name);
			}
		}
	}

	void updateBuildingsList()
	{
		entitiesVisitor([&](Entity *e, const GuiButtonComponent &, const GuiTextComponent &, GuiTextFormatComponent &format) {
			const uint32 name = e->name();
			const uint32 cost = structureMoneyCost(name);
			if (cost == m)
				return;
			format.color = playerMoney < cost ? vec3(1, 0, 0) : name == playerBuildingSelection ? vec3(0, 1, 0) : vec3(1);
		}, engineGui()->entities(), false);
	}

	void engineUpdate()
	{
		updateTopBar();
		updateCursor();
		updateBuildingsList();
	}

	void guiClean()
	{
		engineUpdateListener.detach();
		gameRunning = false;
	}

	bool buildingSelectionClick(uint32 id)
	{
		EntityManager *ents = engineGui()->entities();
		playerBuildingSelection = id;
		return true;
	}

	void generateBuildingButtons(uint32 parent, uint32 ids, PointerRange<const char *const> names)
	{
		EntityManager *ents = engineGui()->entities();
		for (uint32 i = 0; i < names.size(); i++)
		{
			Entity *e = ents->create(ids + i);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = parent;
			pp.order = i;
			GuiButtonComponent &but = e->value<GuiButtonComponent>();
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = names[i];
			GuiTextFormatComponent &format = e->value<GuiTextFormatComponent>();
			GuiEventComponent &evt = e->value<GuiEventComponent>();
			evt.event.bind<&buildingSelectionClick>();
		}
	}

	void generateBuildingsList()
	{
		removeGuiEntitiesWithParent(401);

		EntityManager *ents = engineGui()->entities();

		{
			constexpr const char *names[] = {
				"Wall",
			};
			generateBuildingButtons(401, 900, names);
			buildingSelectionClick(900);
		}

		{
			Entity *e = ents->create(410);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 401;
			pp.order = 1;
			GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
			GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
			ll.vertical = true;
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = "Towers";
		}

		{
			constexpr const char *names[] = {
				"Cheap",
				"Fast",
				"Splash",
				"Sniper",
				"Mage",
			};
			generateBuildingButtons(410, 1000, names);
		}

		{
			Entity *e = ents->create(411);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 401;
			pp.order = 2;
			GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
			GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
			ll.vertical = true;
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = "Augments";
		}

		{
			constexpr const char *names[] = {
				"Fire",
				"Water",
				"Poison",
			};
			generateBuildingButtons(411, 1100, names);
		}

		{
			Entity *e = ents->create(412);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 401;
			pp.order = 3;
			GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
			GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
			ll.vertical = true;
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = "Mana";
		}

		{
			constexpr const char *names[] = {
				"Waterwheel",
				"Sunbloom",
				"Windmill",
				"Snowmelt",
				"Relay",
				"Capacitor",
			};
			generateBuildingButtons(412, 1200, names);
		}

		{
			Entity *e = ents->create(413);
			GuiParentComponent &pp = e->value<GuiParentComponent>();
			pp.parent = 401;
			pp.order = 4;
			GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
			GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
			ll.vertical = true;
			GuiTextComponent &txt = e->value<GuiTextComponent>();
			txt.value = "Traps";
		}

		{
			constexpr const char *names[] = {
				"Spikes",
				"Slowing",
				"Hastening",
			};
			generateBuildingButtons(413, 1300, names);
		}
	}
}

void updateSpawningMonsterPropertiesScreen()
{
	removeGuiEntitiesWithParent(501);

	const SpawningGroup &mo = spawningGroup;
	if (!mo.name)
		return;

	EntityManager *ents = engineGui()->entities();
	sint32 index = 0;

	{ // name
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = stringizer() + SpawningGroup::waveIndex + ": " + mo.name;
	}

	if (mo.monsterClass != MonsterClassFlags::Regular)
	{ // monster class
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Class:";
		struct Pair
		{
			MonsterClassFlags flag = MonsterClassFlags::None;
			StringLiteral name;
		};
		constexpr const Pair pairs[] = {
			Pair{ MonsterClassFlags::Regular, "Regular" },
			Pair{ MonsterClassFlags::Flyer, "Flyer" },
			Pair{ MonsterClassFlags::Boss, "Boss" },
		};
		for (const auto &it : pairs)
			if (any(mo.monsterClass & it.flag))
				txt.value += stringizer() + " " + string(it.name);
	}

	if (mo.immunities != DamageTypeFlags::None)
	{ // immunities
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Immunities:";
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
			Pair{ DamageTypeFlags::Slow, "Slow" },
			Pair{ DamageTypeFlags::Haste, "Haste" },
		};
		for (const auto &it : pairs)
			if (any(mo.immunities & it.flag))
				txt.value += stringizer() + " " + string(it.name);
	}

	{ // amount
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = stringizer() + "Amount: " + (mo.spawnCount * mo.spawnSimultaneously);
	}

	{ // life
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = stringizer() + "Life: " + mo.life;
	}

	{ // speed
		Entity *e = ents->createUnique();
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 501;
		pp.order = index++;
		GuiLabelComponent &lab = e->value<GuiLabelComponent>();
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = stringizer() + "Speed: " + (mo.speed * 20);
	}
}

void setScreenGame()
{
	cleanGui();
	EntityManager *ents = engineGui()->entities();
	guiCleanListener.attach(eventGuiClean());
	guiCleanListener.bind<&guiClean>();
	engineUpdateListener.attach(controlThread().update);
	engineUpdateListener.bind<&engineUpdate>();
	gameRunning = true;

	// cursor

	{
		Entity *e = ents->create(200);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = vec2(0, 1);
	}

	{
		Entity *e = ents->create(201);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 200;
		GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
		sp.collapsed = false;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Cursor";
	}

	// top bar

	{
		Entity *e = ents->create(300);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = vec2(0.5, 0);
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
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "path";
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
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "health";
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
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "dollars";
	}

	// buildings menu

	{
		Entity *e = ents->create(400);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = vec2(1, 0.5);
	}

	{
		Entity *e = ents->create(401);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 400;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
	}

	generateBuildingsList();

	// monster properties

	{
		Entity *e = ents->create(500);
		GuiScrollbarsComponent &sc = e->value<GuiScrollbarsComponent>();
		sc.alignment = vec2(0, 0);
	}

	{
		Entity *e = ents->create(501);
		GuiParentComponent &pp = e->value<GuiParentComponent>();
		pp.parent = 500;
		GuiSpoilerComponent &sp = e->value<GuiSpoilerComponent>();
		sp.collapsed = false;
		GuiLayoutLineComponent &ll = e->value<GuiLayoutLineComponent>();
		ll.vertical = true;
		GuiTextComponent &txt = e->value<GuiTextComponent>();
		txt.value = "Spawning";
	}

	updateSpawningMonsterPropertiesScreen();
}

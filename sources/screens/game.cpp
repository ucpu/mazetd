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
			CAGE_COMPONENT_GUI(Text, txt, e);
			txt.value = stringizer() + globalWaypoints->minFullDistance;
		}

		{ // health
			Entity *e = ents->get(314);
			CAGE_COMPONENT_GUI(Text, txt, e);
			txt.value = stringizer() + playerHealth;
		}

		{ // dollars
			Entity *e = ents->get(316);
			CAGE_COMPONENT_GUI(Text, txt, e);
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
				CAGE_COMPONENT_GUI(Parent, pp, e);
				pp.parent = 201;
				pp.order = index++;
				CAGE_COMPONENT_GUI(Label, lab, e);
				CAGE_COMPONENT_GUI(Text, txt, e);
				txt.value = nm.name;
			}

			if (g->has<ManaStorageComponent>())
			{ // mana
				Entity *e = ents->createUnique();
				CAGE_COMPONENT_GUI(Parent, pp, e);
				pp.parent = 201;
				pp.order = index++;
				CAGE_COMPONENT_GUI(Label, lab, e);
				CAGE_COMPONENT_GUI(Text, txt, e);
				txt.value = stringizer() + "Mana: " + g->value<ManaStorageComponent>().mana + " / " + g->value<ManaStorageComponent>().capacity;
			}

			if (g->has<MonsterComponent>())
			{ // monster
				{ // life
					Entity *e = ents->createUnique();
					CAGE_COMPONENT_GUI(Parent, pp, e);
					pp.parent = 201;
					pp.order = index++;
					CAGE_COMPONENT_GUI(Label, lab, e);
					CAGE_COMPONENT_GUI(Text, txt, e);
					txt.value = stringizer() + "Life: " + g->value<MonsterComponent>().life;
				}
				{ // waypoints
					Entity *e = ents->createUnique();
					CAGE_COMPONENT_GUI(Parent, pp, e);
					pp.parent = 201;
					pp.order = index++;
					CAGE_COMPONENT_GUI(Label, lab, e);
					CAGE_COMPONENT_GUI(Text, txt, e);
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
				CAGE_COMPONENT_GUI(Parent, pp, e);
				pp.parent = 201;
				pp.order = index++;
				CAGE_COMPONENT_GUI(Label, lab, e);
				CAGE_COMPONENT_GUI(Text, txt, e);
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
			CAGE_COMPONENT_GUI(Parent, pp, e);
			pp.parent = parent;
			pp.order = i;
			CAGE_COMPONENT_GUI(Button, but, e);
			CAGE_COMPONENT_GUI(Text, txt, e);
			txt.value = names[i];
			CAGE_COMPONENT_GUI(TextFormat, format, e);
			CAGE_COMPONENT_GUI(Event, evt, e);
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
			CAGE_COMPONENT_GUI(Parent, pp, e);
			pp.parent = 401;
			pp.order = 1;
			CAGE_COMPONENT_GUI(Spoiler, sp, e);
			CAGE_COMPONENT_GUI(LayoutLine, ll, e);
			ll.vertical = true;
			CAGE_COMPONENT_GUI(Text, txt, e);
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
			CAGE_COMPONENT_GUI(Parent, pp, e);
			pp.parent = 401;
			pp.order = 2;
			CAGE_COMPONENT_GUI(Spoiler, sp, e);
			CAGE_COMPONENT_GUI(LayoutLine, ll, e);
			ll.vertical = true;
			CAGE_COMPONENT_GUI(Text, txt, e);
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
			CAGE_COMPONENT_GUI(Parent, pp, e);
			pp.parent = 401;
			pp.order = 3;
			CAGE_COMPONENT_GUI(Spoiler, sp, e);
			CAGE_COMPONENT_GUI(LayoutLine, ll, e);
			ll.vertical = true;
			CAGE_COMPONENT_GUI(Text, txt, e);
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
			CAGE_COMPONENT_GUI(Parent, pp, e);
			pp.parent = 401;
			pp.order = 4;
			CAGE_COMPONENT_GUI(Spoiler, sp, e);
			CAGE_COMPONENT_GUI(LayoutLine, ll, e);
			ll.vertical = true;
			CAGE_COMPONENT_GUI(Text, txt, e);
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
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = stringizer() + SpawningGroup::waveIndex + ": " + mo.name;
	}

	if (mo.monsterClass != MonsterClassFlags::Regular)
	{ // monster class
		Entity *e = ents->createUnique();
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
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
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
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
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = stringizer() + "Amount: " + (mo.spawnCount * mo.spawnSimultaneously);
	}

	{ // life
		Entity *e = ents->createUnique();
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = stringizer() + "Life: " + mo.life;
	}

	{ // speed
		Entity *e = ents->createUnique();
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 501;
		pp.order = index++;
		CAGE_COMPONENT_GUI(Label, lab, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
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
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0, 1);
	}

	{
		Entity *e = ents->create(201);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 200;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		sp.collapsed = false;
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
		ll.vertical = true;
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Cursor";
	}

	// top bar

	{
		Entity *e = ents->create(300);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0.5, 0);
	}

	{
		Entity *e = ents->create(301);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 300;
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
	}

	{
		Entity *e = ents->create(310);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 301;
		pp.order = 1;
		CAGE_COMPONENT_GUI(Button, but, e);
		CAGE_COMPONENT_GUI(Image, img, e);
		img.textureName = HashString("mazetd/gui/menu.png");
		CAGE_COMPONENT_GUI(Event, ev, e);
		ev.event.bind<&buttonMenu>();
	}

	{
		Entity *e = ents->create(302);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 301;
		pp.order = 2;
		CAGE_COMPONENT_GUI(Panel, pnl, e);
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
	}

	{
		Entity *e = ents->create(311);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 302;
		pp.order = 1;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Image, img, e);
		img.textureName = HashString("mazetd/gui/path.png");
	}

	{
		Entity *e = ents->create(312);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 302;
		pp.order = 2;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "path";
	}

	{
		Entity *e = ents->create(303);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 301;
		pp.order = 3;
		CAGE_COMPONENT_GUI(Panel, pnl, e);
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
	}

	{
		Entity *e = ents->create(313);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 303;
		pp.order = 1;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Image, img, e);
		img.textureName = HashString("mazetd/gui/health.png");
	}

	{
		Entity *e = ents->create(314);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 303;
		pp.order = 2;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "health";
	}

	{
		Entity *e = ents->create(304);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 301;
		pp.order = 4;
		CAGE_COMPONENT_GUI(Panel, pnl, e);
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
	}

	{
		Entity *e = ents->create(315);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 304;
		pp.order = 1;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Image, img, e);
		img.textureName = HashString("mazetd/gui/dollar.png");
	}

	{
		Entity *e = ents->create(316);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 304;
		pp.order = 2;
		CAGE_COMPONENT_GUI(Label, lb, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "dollars";
	}

	// buildings menu

	{
		Entity *e = ents->create(400);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(1, 0.5);
	}

	{
		Entity *e = ents->create(401);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 400;
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
		ll.vertical = true;
	}

	generateBuildingsList();

	// monster properties

	{
		Entity *e = ents->create(500);
		CAGE_COMPONENT_GUI(Scrollbars, sc, e);
		sc.alignment = vec2(0, 0);
	}

	{
		Entity *e = ents->create(501);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 500;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		sp.collapsed = false;
		CAGE_COMPONENT_GUI(LayoutLine, ll, e);
		ll.vertical = true;
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Spawning";
	}

	updateSpawningMonsterPropertiesScreen();
}

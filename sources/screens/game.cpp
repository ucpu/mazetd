#include <cage-core/hashString.h>
#include <cage-core/entitiesVisitor.h>

#include "screens.h"
#include "../grid.h"
#include "../game.h"

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

		entitiesVisitor(gameEntities(), [&](Entity *g, const PositionComponent &po, const NameComponent &nm) {
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
		});

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

	void engineUpdate()
	{
		updateTopBar();
		updateCursor();
	}

	void guiClean()
	{
		engineUpdateListener.detach();
		gameRunning = false;
	}

	bool buildingSelectionClick(uint32 id)
	{
		EntityManager *ents = engineGui()->entities();
		if (playerBuildingSelection != 0 && ents->has(playerBuildingSelection))
		{
			Entity *e = ents->get(playerBuildingSelection);
			e->remove(ents->component<GuiTextFormatComponent>());
		}
		playerBuildingSelection = id;
		{
			Entity *e = ents->get(playerBuildingSelection);
			CAGE_COMPONENT_GUI(TextFormat, tf, e);
			tf.color = vec3(1, 0, 0);
		}
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
			CAGE_COMPONENT_GUI(Event, evt, e);
			evt.event.bind<&buildingSelectionClick>();
		}
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
		txt.value = "Magic";
	}

	{
		constexpr const char *names[] = {
			"Mage Tower",
			"Waterwheel",
			"Sunbloom",
			"Windmill",
			"Snowmill",
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

#include <cage-core/hashString.h>

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
			txt.value = stringizer() + globalWaypoints->avgFullDistance;
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
		// todo
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
		CAGE_COMPONENT_GUI(Panel, pa, e);
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
		Entity *e = ents->create(410);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 401;
		pp.order = 0;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Damage";
	}

	{
		Entity *e = ents->create(411);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 401;
		pp.order = 1;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Support";
	}

	{
		Entity *e = ents->create(412);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 401;
		pp.order = 2;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Traps";
	}

	{
		Entity *e = ents->create(413);
		CAGE_COMPONENT_GUI(Parent, pp, e);
		pp.parent = 401;
		pp.order = 3;
		CAGE_COMPONENT_GUI(Spoiler, sp, e);
		CAGE_COMPONENT_GUI(Text, txt, e);
		txt.value = "Utility";
	}
}

#include "../game.h"
#include "../grid.h"
#include "screens.h"

#include <cage-core/entitiesVisitor.h>
#include <cage-core/hashString.h>

namespace mazetd
{
	void setScreenPaused();
	void generateBuildingsList();

	namespace
	{
		EventListener<bool()> engineUpdateListener;
		EventListener<bool()> guiCleanListener;

		void updateTopBar()
		{
			EntityManager *ents = engineGuiEntities();

			{ // path tiles
				ents->get(312)->value<GuiTextComponent>().value = Stringizer() + globalWaypoints->minFullDistance;
			}

			{ // health
				ents->get(314)->value<GuiTextComponent>().value = Stringizer() + playerHealth;
			}

			{ // dollars
				ents->get(316)->value<GuiTextComponent>().value = Stringizer() + playerMoney;
			}

			{ // mana
				sint64 manaCap = 1;
				sint64 manaAvail = 0;
				entitiesVisitor(
					[&](const ManaStorageComponent &mc)
					{
						manaCap += mc.capacity;
						manaAvail += mc.mana;
					},
					gameEntities(), false);
				ents->get(318)->value<GuiTextComponent>().value = Stringizer() + (100 * manaAvail / manaCap) + " %";
			}

			{ // monsters
				uint32 cnt = 0;
				sint64 lfTot = 1;
				sint64 lfRem = 0;
				entitiesVisitor(
					[&](const MonsterComponent &mc)
					{
						cnt++;
						lfTot += mc.maxLife;
						lfRem += mc.life;
					},
					gameEntities(), false);
				ents->get(320)->value<GuiTextComponent>().value = Stringizer() + cnt + " @ " + (100 * lfRem / lfTot) + " %";
			}
		}

		void guiClean()
		{
			engineUpdateListener.detach();
			gameReady = false;
		}
	}

	void updateSpawningMonsterPropertiesScreen()
	{
		detail::guiDestroyEntityRecursively(engineGuiEntities()->get(502));
		Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities()->get(501));
		auto _1 = g->setNextName(502).empty();
		auto _2 = g->column();

		const SpawningGroup &mo = spawningGroup;
		if (!mo.name)
			return;

		// name
		g->label().text(Stringizer() + SpawningGroup::waveIndex + ": " + mo.name);

		// monster class
		if (mo.monsterClass != MonsterClassFlags::None)
		{
			Stringizer txt;
			txt + "Class:";
			struct Pair
			{
				MonsterClassFlags flag = MonsterClassFlags::None;
				StringPointer name;
			};
			static constexpr const Pair pairs[] = {
				Pair{ MonsterClassFlags::Flier, "Flier" },
				Pair{ MonsterClassFlags::Boss, "Boss" },
			};
			for (const auto &it : pairs)
				if (any(mo.monsterClass & it.flag))
					txt + " " + String(it.name);
			g->label().text(txt);
		}

		// resistances
		if (mo.resistances != DamageTypeFlags::None)
		{
			Stringizer txt;
			txt + "Resistances:";
			struct Pair
			{
				DamageTypeFlags flag = DamageTypeFlags::None;
				StringPointer name;
			};
			static constexpr const Pair pairs[] = {
				Pair{ DamageTypeFlags::Physical, "Physical" },
				Pair{ DamageTypeFlags::Fire, "Fire" },
				Pair{ DamageTypeFlags::Water, "Water" },
				Pair{ DamageTypeFlags::Poison, "Poison" },
				Pair{ DamageTypeFlags::Magic, "Magic" },
			};
			for (const auto &it : pairs)
				if (any(mo.resistances & it.flag))
					txt + " " + String(it.name);
			g->label().text(txt);
		}

		// count
		g->label().text(Stringizer() + "Count: " + (mo.spawnCount * mo.spawnSimultaneously));

		// life
		g->label().text(Stringizer() + "Life: " + mo.maxLife);

		// speed
		g->label().text(Stringizer() + "Speed: " + mo.speed);

		// money
		g->label().text(Stringizer() + "Reward: " + mo.money);
	}

	void setScreenGame()
	{
		cleanGui();
		guiCleanListener.attach(eventGuiClean());
		guiCleanListener.bind(&guiClean);
		engineUpdateListener.attach(controlThread().update);
		engineUpdateListener.bind(&updateTopBar);
		gameReady = true;
		Holder<GuiBuilder> g = newGuiBuilder(engineGuiEntities());

		{ // top bar
			auto _1 = g->alignment(Vec2(0.5, 0));
			auto _2 = g->row();
			g->button().image(HashString("mazetd/gui/menu.png")).event(setScreenPaused).size(Vec2(50, 0));
			{
				auto _1 = g->panel();
				auto _2 = g->row();
				g->label().image(HashString("mazetd/gui/path.png"));
				g->setNextName(312).label();
			}
			{
				auto _1 = g->panel();
				auto _2 = g->row();
				g->label().image(HashString("mazetd/gui/health.png"));
				g->setNextName(314).label();
			}
			{
				auto _1 = g->panel();
				auto _2 = g->row();
				g->label().image(HashString("mazetd/gui/dollar.png"));
				g->setNextName(316).label();
			}
			{
				auto _1 = g->panel();
				auto _2 = g->row();
				g->label().image(HashString("mazetd/gui/mana.png"));
				g->setNextName(318).label();
			}
			{
				auto _1 = g->panel();
				auto _2 = g->row();
				g->label().image(HashString("mazetd/gui/monster.png"));
				g->setNextName(320).label();
			}
			updateTopBar();
		}

		{ // buildings menu
			auto _1 = g->alignment(Vec2(1, 0.5));
			auto _2 = g->setNextName(401).column();
			generateBuildingsList();
		}

		{ // monster properties
			auto _1 = g->alignment(Vec2(0, 0));
			auto _2 = g->spoiler(false).text("Spawning").skin(2); // compact skin
			auto _3 = g->setNextName(501).empty();
			g->setNextName(502).empty();
			updateSpawningMonsterPropertiesScreen();
		}

		{ // controls
			auto _1 = g->alignment(Vec2(0, 1));
			auto _2 = g->spoiler(false).text("Controls").skin(2); // compact skin
			auto _3 = g->column();

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

			for (const String &it : lines)
				g->label().text(it);
		}
	}
}

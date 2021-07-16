#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>

namespace
{
	struct Sorting
	{
		Entity *e = nullptr;
		vec3 p; // position
		real d; // distance to the tower
	};

	Sorting sorting(uint32 name, const vec3 &towerPosition)
	{
		Sorting m;
		m.e = gameEntities()->get(name);
		m.p = m.e->has<MovementComponent>() ? m.e->value<MovementComponent>().position() : globalGrid->center(m.e->value<PositionComponent>().tile);
		m.d = distance(m.p, towerPosition);
		return m;
	}

	struct Monster : public Sorting
	{
		MonsterComponent *m = nullptr;
	};

	struct Augment : public Sorting
	{
		AugmentComponent *a = nullptr;
	};

	Augment findAugment(const AttackComponent &atc, AttackData &att, SpatialQuery *buildingsQuery, const vec3 &mp)
	{
		buildingsQuery->intersection(Sphere(mp, 5));
		std::vector<Augment> augs;
		augs.reserve(buildingsQuery->result().size());
		for (uint32 n : buildingsQuery->result())
		{
			Augment a;
			(Sorting &)a = sorting(n, mp);
			if (!a.e->has<AugmentComponent>())
				continue;
			a.a = &a.e->value<AugmentComponent>();
			augs.push_back(a);
		}
		if (augs.empty())
			return {};

		std::partial_sort(augs.begin(), augs.begin() + 1, augs.end(), [](const Augment &a, const Augment &b) {
			return a.d < b.d;
		});

		const Augment &a = augs[0];
		att = atc.data[(int)a.a->data];

		return a;
	}

	void filterMonsterTargets(const AttackData &att, SpatialQuery *monstersQuery, std::vector<Monster> &monsters, const vec3 &mp)
	{
		monsters.clear();
		monsters.reserve(monstersQuery->result().size());
		for (uint32 n : monstersQuery->result())
		{
			Monster m;
			(Sorting &)m = sorting(n, mp);
			m.m = &m.e->value<MonsterComponent>();
			if (none(att.damageType & ~m.m->immunities))
				continue;
			if (any(att.invalidClasses & m.m->monsterClass))
				continue;
			monsters.push_back(m);
		}
	}

	bool findTargetMonsters(const AttackData &att, SpatialQuery *monstersQuery, std::vector<Monster> &monsters, const vec3 &mp)
	{
		monstersQuery->intersection(Sphere(mp, att.firingRange));
		if (monstersQuery->result().empty())
			return false;

		filterMonsterTargets(att, monstersQuery, monsters, mp);
		if (monsters.empty())
			return false;

		std::partial_sort(monsters.begin(), monsters.begin() + 1, monsters.end(), [](const Monster &a, const Monster &b) {
			return a.m->timeToArrive < b.m->timeToArrive;
		});
		return true;
	}

	bool checkManaCost(const AttackData &att, Entity *e)
	{
		if (att.manaCost > 0)
		{
			e->value<ManaReceiverComponent>();
			ManaStorageComponent &mn = e->value<ManaStorageComponent>();
			if (mn.mana < att.manaCost)
				return false;
			mn.mana -= att.manaCost;
		}
		else
			e->remove<ManaReceiverComponent>();
		return true;
	}

	void effectAugment(const AttackData &att, const Augment &augment, const vec3 &mpp)
	{
		if (!augment.e)
			return;

		// effect from the augment to the tower
		EffectConfig cfg;
		cfg.pos1 = augment.p + vec3(0, augment.e->value<PivotComponent>().elevation, 0);
		cfg.pos2 = mpp;
		cfg.type = att.effectType;
		renderEffect(cfg);
	}

	void effectMonster(const AttackData &att, const Monster &m, const vec3 &mpp)
	{
		// effect from the tower to the monster
		EffectConfig cfg;
		cfg.pos1 = mpp;
		cfg.pos2 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
		cfg.type = att.effectType;
		renderEffect(cfg);
	}

	void applySplash(const AttackData &att, SpatialQuery *monstersQuery, std::vector<Monster> &monsters, const vec3 &mp)
	{
		if (att.splashRadius > 0)
		{
			// search for all monsters inside the splash radius
			monstersQuery->intersection(Sphere(monsters[0].p, att.splashRadius));
			CAGE_ASSERT(!monstersQuery->result().empty());
			filterMonsterTargets(att, monstersQuery, monsters, mp);
		}
		else
		{
			// single target - erase other monsters
			monsters.erase(monsters.begin() + 1, monsters.end());
		}
	}

	void attackMonster(const Monster &m, const AttackData &att)
	{
		constexpr DamageTypeFlags attackTypes = DamageTypeFlags::Physical | DamageTypeFlags::Fire | DamageTypeFlags::Water | DamageTypeFlags::Poison | DamageTypeFlags::Magic;
		if (any(att.damageType & attackTypes & ~m.m->immunities))
			m.m->life -= att.damage;

		constexpr DamageTypeFlags debuffTypes = DamageTypeFlags::Slow | DamageTypeFlags::Haste;
		const DamageTypeFlags debuffs = att.damageType & debuffTypes & ~m.m->immunities;
		if (any(debuffs))
		{
			MonsterDebuffComponent &md = m.e->value<MonsterDebuffComponent>();
			md.type = debuffs;
			md.endTime = gameTime + 30 * 5;
		}
	}

	void effectMonster(const Monster &m, const AttackData &att)
	{
		// effect around the monster
		EffectConfig cfg;
		cfg.pos1 = m.p + vec3(0, m.e->value<PivotComponent>().elevation, 0);
		for (uint32 i = 0; i < 3; i++)
		{
			vec3 d = randomDirection3();
			d[1] = abs(d[1]);
			cfg.pos2 = cfg.pos1 + d * 0.5;
			cfg.type = att.effectType;
			renderEffect(cfg);
		}
	}

	void engineUpdate()
	{
		if (!gameRunning)
			return;

		SpatialQuery *monstersQuery = spatialMonsters();
		SpatialQuery *buildingsQuery = spatialStructures();
		std::vector<Monster> monsters;

		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, AttackComponent &atc) {
			if (atc.firingDelay > 0)
			{
				atc.firingDelay--;
				return;
			}

			const vec3 mp = globalGrid->center(pos.tile);
			const vec3 mpp = mp + vec3(0, e->value<PivotComponent>().elevation, 0);
			AttackData att = atc.data[0];
			Augment augment = atc.useAugments ? findAugment(atc, att, buildingsQuery, mp) : Augment();
			if (!findTargetMonsters(att, monstersQuery, monsters, mp))
				return;
			if (!checkManaCost(att, e))
				return;
			effectAugment(att, augment, mpp);
			effectMonster(att, monsters[0], mpp);
			applySplash(att, monstersQuery, monsters, mp);
			for (const Monster &m : monsters)
			{
				attackMonster(m, att);
				effectMonster(m, att);
			}

			atc.firingDelay += att.firingPeriod;
		});
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, 50);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

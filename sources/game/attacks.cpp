#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/geometry.h>
#include <cage-core/enumerate.h>
#include <cage-engine/scene.h>

#include "../game.h"
#include "../grid.h"

#include <vector>
#include <algorithm>

namespace
{
	struct Neighbor
	{
		Vec3 np;
		Vec3 npp;
		Real dist2;
		Entity *e = nullptr;
	};

	struct TowerData : public DamageComponent
	{
		Entity *me = nullptr;
		Vec3 mp;
		Vec3 mpp;

		BonusTypeEnum bonusType = BonusTypeEnum::None;
		const Neighbor *bonusNeighbor = nullptr;

		DamageTypeEnum damageType = DamageTypeEnum::Physical;
		const Neighbor *elementNeighbor = nullptr;

		TargetingEnum targetingType = TargetingEnum::Random;
		const Neighbor *targetingNeighbor = nullptr;

		uint32 manaCost = 0;
	};

	struct Monster
	{
		Entity *e = nullptr;
		MonsterComponent *mc = nullptr;
		Vec3 p;
	};

	template<TargetingEnum Targeting>
	struct MonsterPicker;

	struct AttacksSolver : public TowerData
	{
		SpatialQuery *monstersQuery = spatialMonsters();
		SpatialQuery *buildingsQuery = spatialStructures();
		EntityComponent *compDamage = gameEntities()->component<DamageComponent>();
		EntityComponent *compModBonus = gameEntities()->component<ModBonusComponent>();
		EntityComponent *compModElement = gameEntities()->component<ModElementComponent>();
		EntityComponent *compModTargeting = gameEntities()->component<ModTargetingComponent>();
		EntityComponent *compMovement = gameEntities()->component<MovementComponent>();
		EntityComponent *compPosition = gameEntities()->component<PositionComponent>();
		EntityComponent *compPivot = gameEntities()->component<PivotComponent>();
		EntityComponent *compManaRecv = gameEntities()->component<ManaReceiverComponent>();
		EntityComponent *compManaStor = gameEntities()->component<ManaStorageComponent>();
		std::vector<Neighbor> neighbors;
		std::vector<Monster> monsters;

		void findNeighbors()
		{
			neighbors.clear();
			buildingsQuery->intersection(Sphere(mp, 5));
			for (uint32 nn : buildingsQuery->result())
			{
				if (nn == me->name())
					continue;
				Neighbor n;
				n.e = gameEntities()->get(nn);
				n.np = globalGrid->center(n.e->value<PositionComponent>().tile);
				n.npp = n.np + Vec3(0, n.e->value<PivotComponent>().elevation, 0);
				n.dist2 = distanceSquared(n.np, mp);
				neighbors.push_back(n);
			}
			std::sort(neighbors.begin(), neighbors.end(), [](const Neighbor &a, const Neighbor &b) {
				return a.dist2 < b.dist2;
			});
		}

		const Neighbor *closestNeighbor(EntityComponent *comp)
		{
			for (const auto &it : neighbors)
			{
				if (it.e->has(comp))
					return &it;
			}
			return nullptr;
		}

		void findMods()
		{
			bonusNeighbor = closestNeighbor(compModBonus);
			if (bonusNeighbor)
				bonusType = bonusNeighbor->e->value<ModBonusComponent>(compModBonus).type;

			elementNeighbor = closestNeighbor(compModElement);
			if (elementNeighbor)
				damageType = elementNeighbor->e->value<ModElementComponent>(compModElement).element;

			targetingNeighbor = closestNeighbor(compModTargeting);
			if (targetingNeighbor)
				targetingType = targetingNeighbor->e->value<ModTargetingComponent>(compModTargeting).targeting;
		}

		void applyBonus()
		{
			if (elementNeighbor)
			{
				CAGE_ASSERT(overTime == 0);
				overTime = numeric_cast<uint32>(cage::sqrt(damage) * 20);
				CAGE_ASSERT(manaCost == 0);
				manaCost = baseManaCost;
				damage *= 3; // cost of original damage + cost of mana + cost of DOT
			}
			switch (bonusType)
			{
			case BonusTypeEnum::Damage: damage *= 2; manaCost *= 2; break;
			case BonusTypeEnum::FiringRate: firingPeriod /= 2; break;
			case BonusTypeEnum::FiringRange: firingRange += 4; break;
			case BonusTypeEnum::SplashRadius: splashRadius += 2; manaCost *= 3; break;
			case BonusTypeEnum::IntenseDot: overTime /= 5; break;
			case BonusTypeEnum::ManaDiscount: manaCost /= 2; break;
			}
		}

		void filterMonsters()
		{
			const DamageTypeFlags dmgFlags = DamageTypeFlags(1u << (uint32)damageType);
			monsters.clear();
			for (uint32 n : monstersQuery->result())
			{
				Monster mo;
				mo.e = gameEntities()->get(n);
				mo.p = mo.e->has(compMovement) ? mo.e->value<MovementComponent>(compMovement).position() : mo.e->value<PositionComponent>(compPosition).position();
				mo.mc = &mo.e->value<MonsterComponent>();
				if (none(dmgFlags & ~mo.mc->immunities))
					continue;
				if (any(invalidClasses & mo.mc->monsterClass))
					continue;
				monsters.push_back(mo);
			}
		}

		void findMonstersForTower()
		{
			monstersQuery->intersection(Sphere(mp, firingRange));
			filterMonsters();
		}

		void findMonstersForSplash()
		{
			monstersQuery->intersection(Sphere(monsters[0].p, splashRadius));
			filterMonsters();
		}

		void pickTargetMonster();

		bool consumeMana()
		{
			if (manaCost > 0)
			{
				me->add(compManaRecv);
				ManaStorageComponent &mn = me->value<ManaStorageComponent>(compManaStor);
				mn.capacity = baseManaCost * 10;
				if (mn.mana < manaCost)
					return false;
				mn.mana -= manaCost;
			}
			else
				me->remove(compManaRecv);
			return true;
		}

		void effects()
		{
			EffectConfig cfg;
			cfg.type = damageType;
			cfg.pos2 = mpp;
			if (bonusNeighbor)
			{
				cfg.pos1 = bonusNeighbor->npp;
				renderEffect(cfg);
			}
			if (elementNeighbor)
			{
				cfg.pos1 = elementNeighbor->npp;
				renderEffect(cfg);
			}
			if (targetingNeighbor)
			{
				cfg.pos1 = targetingNeighbor->npp;
				renderEffect(cfg);
			}
			cfg.pos1 = mpp;
			cfg.pos2 = monsters[0].p + Vec3(0, monsters[0].e->value<PivotComponent>(compPivot).elevation, 0);
			renderEffect(cfg);
		}

		void damageMonsters()
		{
			CAGE_ASSERT(damageType < DamageTypeEnum::Total);
			for (const auto &it : monsters)
			{
				auto &dot = it.mc->dots[(uint32)damageType];
				dot.damage += damage;
				dot.duration += overTime;
			}
		}

		void run()
		{
			entitiesVisitor([&](Entity *e, const PositionComponent &pos, AttackComponent &atc) {
				if (atc.firingDelay > 0)
				{
					atc.firingDelay--;
					return;
				}

				*(TowerData *)this = {};
				*(DamageComponent *)this = e->value<DamageComponent>(compDamage);
				me = e;
				mp = pos.position();
				mpp = mp + Vec3(0, e->value<PivotComponent>(compPivot).elevation, 0);

				if (acceptMods)
				{
					findNeighbors();
					findMods();
					applyBonus();
				}

				findMonstersForTower();
				if (monsters.empty())
					return;
				if (!consumeMana())
					return;
				pickTargetMonster();
				effects();
				if (splashRadius > 0)
					findMonstersForSplash();
				damageMonsters();

				atc.firingDelay += firingPeriod;
			}, gameEntities(), false);
		}
	};

	template<TargetingEnum Targeting>
	struct MonsterPicker
	{
		AttacksSolver *data = nullptr;

		MonsterPicker(AttacksSolver *data) : data(data)
		{
			CAGE_ASSERT(data->targetingType == Targeting);
		}

		Real value(Monster &mo)
		{
			switch (Targeting)
			{
			case TargetingEnum::Back: return -numeric_cast<sint32>(mo.mc->timeToArrive);
			case TargetingEnum::Closest: return distanceSquared(mo.p, data->mp);
			case TargetingEnum::Farthest: return -distanceSquared(mo.p, data->mp);
			case TargetingEnum::Front: return mo.mc->timeToArrive;
			case TargetingEnum::Strongest: return -mo.mc->life;
			case TargetingEnum::Weakest: return mo.mc->life;
			default: { CAGE_THROW_CRITICAL(NotImplemented, "invalid targeting"); } break;
			}
		}

		void run()
		{
			uint32 bi = 0;
			Real bv = value(data->monsters[0]);
			for (const auto &it : enumerate(data->monsters))
			{
				Real v = value(*it);
				if (v < bv)
				{
					bi = it.index;
					bv = v;
				}
			}
			data->monsters[0] = data->monsters[bi];
			data->monsters.resize(1);
		}
	};

	template<>
	struct MonsterPicker<TargetingEnum::Random>
	{
		AttacksSolver *data = nullptr;

		MonsterPicker(AttacksSolver *data) : data(data)
		{
			CAGE_ASSERT(data->targetingType == TargetingEnum::Random);
		}

		void run()
		{
			data->monsters[0] = data->monsters[randomRange(uintPtr(), data->monsters.size())];
			data->monsters.resize(1);
		}
	};

	void AttacksSolver::pickTargetMonster()
	{
		CAGE_ASSERT(monsters.size() > 0);
		switch (targetingType)
		{
		case TargetingEnum::Back: { MonsterPicker<TargetingEnum::Back>(this).run(); } break;
		case TargetingEnum::Closest: { MonsterPicker<TargetingEnum::Closest>(this).run(); } break;
		case TargetingEnum::Farthest: { MonsterPicker<TargetingEnum::Farthest>(this).run(); } break;
		case TargetingEnum::Front: { MonsterPicker<TargetingEnum::Front>(this).run(); } break;
		case TargetingEnum::Random: { MonsterPicker<TargetingEnum::Random>(this).run(); } break;
		case TargetingEnum::Strongest: { MonsterPicker<TargetingEnum::Strongest>(this).run(); } break;
		case TargetingEnum::Weakest: { MonsterPicker<TargetingEnum::Weakest>(this).run(); } break;
		default: { CAGE_THROW_CRITICAL(NotImplemented, "invalid targeting"); } break;
		}
		CAGE_ASSERT(monsters.size() == 1);
	}

	void gameUpdate()
	{
		AttacksSolver solver;
		solver.run();
	}

	struct Callbacks
	{
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameUpdateListener.attach(eventGameUpdate(), 50);
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

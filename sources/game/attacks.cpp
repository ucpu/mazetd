#include <algorithm>
#include <vector>

#include "../game.h"
#include "../grid.h"

#include <cage-core/entitiesVisitor.h>
#include <cage-core/enumerate.h>
#include <cage-core/geometry.h>
#include <cage-core/profiling.h>
#include <cage-core/spatialStructure.h>
#include <cage-engine/scene.h>

namespace mazetd
{
	namespace
	{
		struct TowerData
		{
			Entity *me = nullptr;
			Vec3 mp;
			Vec3 mpp;
		};

		struct Monster
		{
			Entity *e = nullptr;
			MonsterComponent *mc = nullptr;
			Vec3 p;
		};

		template<TargetingEnum Targeting>
		struct MonsterPicker;

		struct AttacksSolver : public TowerData, public DamageComponent, public AttackComponent
		{
			SpatialQuery *monstersQuery = spatialMonsters();
			std::vector<Monster> monsters;

			void applyMods()
			{
				if (element != DamageTypeEnum::Physical)
				{
					CAGE_ASSERT(overTime == 0);
					overTime = numeric_cast<uint32>(cage::sqrt(damage) * 20);
					damage *= 3; // cost of original damage + cost of mana + cost of DOT
				}
				switch (bonus)
				{
					case BonusTypeEnum::Damage:
						damage *= 2;
						manaCost *= 2;
						break;
					case BonusTypeEnum::FiringRate:
						firingPeriod /= 2;
						break;
					case BonusTypeEnum::FiringRange:
						firingRange += 4;
						break; // keep in sync with visualization
					case BonusTypeEnum::SplashRadius:
						splashRadius += 2;
						manaCost *= 3;
						break;
					case BonusTypeEnum::IntenseDot:
						overTime /= 5;
						break;
					case BonusTypeEnum::ManaDiscount:
						manaCost /= 3;
						break;
				}
			}

			void filterMonsters()
			{
				monsters.clear();
				for (uint32 n : monstersQuery->result())
				{
					Monster mo;
					mo.e = gameEntities()->get(n);
					mo.p = mo.e->has<MovementComponent>() ? mo.e->value<MovementComponent>().position() : mo.e->value<PositionComponent>().position();
					mo.mc = &mo.e->value<MonsterComponent>();
					if (any(invalidClasses & mo.mc->monsterClass))
						continue;
					monsters.push_back(mo);
				}
			}

			void findMonstersForTower()
			{
				monstersQuery->intersection(Sphere(mp * Vec3(1, 0, 1), firingRange));
				filterMonsters();
			}

			void findMonstersForSplash()
			{
				monstersQuery->intersection(Sphere(monsters[0].p * Vec3(1, 0, 1), splashRadius));
				filterMonsters();
			}

			void pickTargetMonster();

			bool consumeMana()
			{
				if (manaCost > 0)
				{
					ManaStorageComponent &mn = me->value<ManaStorageComponent>();
					if (mn.mana < manaCost)
						return false;
					mn.mana -= manaCost;
				}
				return true;
			}

			void effects()
			{
				EffectConfig cfg;
				cfg.type = element;
				cfg.pos1 = mpp;
				cfg.pos2 = monsters[0].p + Vec3(0, monsters[0].e->value<PivotComponent>().elevation, 0);
				renderEffect(cfg);
			}

			void damageMonsters()
			{
				CAGE_ASSERT(element < DamageTypeEnum::Total);
				for (const auto &it : monsters)
				{
					auto &dot = it.mc->dots[(uint32)element];
					dot.damage += damage;
					dot.duration += overTime;
				}
			}

			void run()
			{
				entitiesVisitor(
					[&](Entity *e, const PositionComponent &pos, const DamageComponent &dmg, AttackComponent &atc)
					{
						if (atc.firingDelay > 0)
						{
							atc.firingDelay--;
							return;
						}

						*(TowerData *)this = {};
						*(DamageComponent *)this = dmg;
						*(AttackComponent *)this = atc;
						me = e;
						mp = pos.position();
						mpp = mp + Vec3(0, e->value<PivotComponent>().elevation, 0);

						applyMods();

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
					},
					gameEntities(), false);
			}
		};

		template<TargetingEnum Targeting>
		struct MonsterPicker
		{
			AttacksSolver *data = nullptr;

			MonsterPicker(AttacksSolver *data) : data(data) { CAGE_ASSERT(data->targeting == Targeting); }

			Real value(Monster &mo)
			{
				switch (Targeting)
				{
					case TargetingEnum::Back:
						return -numeric_cast<sint32>(mo.mc->timeToArrive);
					case TargetingEnum::Closest:
						return distanceSquared(mo.p * Vec3(1, 0, 1), data->mp * Vec3(1, 0, 1));
					case TargetingEnum::Farthest:
						return -distanceSquared(mo.p * Vec3(1, 0, 1), data->mp * Vec3(1, 0, 1));
					case TargetingEnum::Front:
						return mo.mc->timeToArrive;
					case TargetingEnum::Strongest:
						return -mo.mc->life;
					case TargetingEnum::Weakest:
						return mo.mc->life;
					default:
						CAGE_THROW_CRITICAL(Exception, "invalid targeting");
						break;
				}
			}

			void run()
			{
				uint32 bi = 0;
				Real bv = value(data->monsters[0]);
				for (const auto &it : enumerate(data->monsters))
				{
					const Real v = value(*it);
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

			MonsterPicker(AttacksSolver *data) : data(data) { CAGE_ASSERT(data->targeting == TargetingEnum::Random); }

			void run()
			{
				data->monsters[0] = data->monsters[randomRange(uintPtr(), data->monsters.size())];
				data->monsters.resize(1);
			}
		};

		void AttacksSolver::pickTargetMonster()
		{
			CAGE_ASSERT(monsters.size() > 0);
			switch (targeting)
			{
				case TargetingEnum::Back:
				{
					MonsterPicker<TargetingEnum::Back>(this).run();
					break;
				}
				case TargetingEnum::Closest:
				{
					MonsterPicker<TargetingEnum::Closest>(this).run();
					break;
				}
				case TargetingEnum::Farthest:
				{
					MonsterPicker<TargetingEnum::Farthest>(this).run();
					break;
				}
				case TargetingEnum::Front:
				{
					MonsterPicker<TargetingEnum::Front>(this).run();
					break;
				}
				case TargetingEnum::Random:
				{
					MonsterPicker<TargetingEnum::Random>(this).run();
					break;
				}
				case TargetingEnum::Strongest:
				{
					MonsterPicker<TargetingEnum::Strongest>(this).run();
					break;
				}
				case TargetingEnum::Weakest:
				{
					MonsterPicker<TargetingEnum::Weakest>(this).run();
					break;
				}
				default:
					CAGE_THROW_CRITICAL(Exception, "invalid targeting");
					break;
			}
			CAGE_ASSERT(monsters.size() == 1);
		}

		const auto gameUpdateListener = eventGameUpdate().listen(
			[]()
			{
				const ProfilingScope profiling("attacks");
				AttacksSolver solver;
				solver.run();
			},
			50);
	}
}

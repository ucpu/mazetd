#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

void setScreenLost();

Vec3 PositionComponent::position() const
{
	return globalGrid->center(tile);
}

Vec3 MovementComponent::position() const
{
	const Vec3 ca = globalGrid->center(tileStart);
	const Vec3 cb = globalGrid->center(tileEnd);
	const Real fac = saturate(Real(gameTime - (sint64)timeStart) / Real(timeEnd - (sint64)timeStart));
	return interpolate(ca, cb, fac);
}

namespace
{
	void registerEntityComponents(EntityManager *man)
	{
		man->defineComponent(NameComponent());
		man->defineComponent(DescriptionComponent());
		man->defineComponent(BuildingComponent());
		man->defineComponent(TrapComponent());
		man->defineComponent(PivotComponent());
		man->defineComponent(CostComponent());
		man->defineComponent(ManaStorageComponent());
		man->defineComponent(ManaDistributorComponent());
		man->defineComponent(ManaReceiverComponent());
		man->defineComponent(ManaCollectorComponent());
		man->defineComponent(DamageComponent());
		man->defineComponent(ModElementComponent());
		man->defineComponent(ModBonusComponent());
		man->defineComponent(ModTargetingComponent());
	}

	Holder<EntityManager> initializeManager()
	{
		Holder<EntityManager> man = newEntityManager();
		registerEntityComponents(+man);
		man->defineComponent(EngineComponent());
		man->defineComponent(HealthbarComponent());
		man->defineComponent(ManabarComponent());
		man->defineComponent(PositionComponent());
		man->defineComponent(MovementComponent());
		man->defineComponent(MonsterComponent());
		man->defineComponent(AttackComponent());
		return man;
	}

	const auto engineInitListener = controlThread().initialize.listen([]() {
		registerEntityComponents(engineGuiEntities());
		engineGuiEntities()->defineComponent(GuiModelComponent());
		eventGameReset().dispatch();
	}, 200);

	const auto gameResetListener = eventGameReset().listen([]() {
		gameTime = 0;
		gameSpeed = 1;
		gameReady = false;
		gamePaused = false;
		gameEntities()->destroy();

		playerCursorPosition = Vec3::Nan();
		playerCursorTile = m;
		playerHealth = 100;
		playerMoney = 1000;
		playerBuildingSelection = nullptr;
	}, -100);

	const auto gameUpdateListener = eventGameUpdate().listen([]() {
		gameTime++;
		if (playerHealth <= 0)
		{
			setScreenLost();
			return true;
		}
		return false;
	}, -500);
}

EntityManager *gameEntities()
{
	static Holder<EntityManager> man = initializeManager();
	return +man;
}

EventDispatcher<bool()> &eventGameReset()
{
	static EventDispatcher<bool()> e;
	return e;
}

EventDispatcher<bool()> &eventGameUpdate()
{
	static EventDispatcher<bool()> e;
	return e;
}

uint32 gameTime = 0;
Real gameSpeed = 1;
bool gameReady = false;
bool gamePaused = false;

Vec3 playerCursorPosition;
uint32 playerCursorTile;
sint32 playerHealth;
sint32 playerMoney;
bool playerPanning = false;
Entity *playerBuildingSelection;
bool playerWon = false;

#include <cage-engine/engine.h>

#include "../game.h"

void setScreenGameOver();

namespace
{
	Holder<EntityManager> initializeManager()
	{
		Holder<EntityManager> man = newEntityManager();
		man->defineComponent(PositionComponent());
		man->defineComponent(MovementComponent());
		man->defineComponent(PivotComponent());
		man->defineComponent(NameComponent());
		man->defineComponent(BuildingComponent());
		man->defineComponent(TrapComponent());
		man->defineComponent(RefundCostComponent());
		man->defineComponent(ManaStorageComponent());
		man->defineComponent(ManaDistributorComponent());
		man->defineComponent(ManaReceiverComponent());
		man->defineComponent(ManaCollectorComponent());
		man->defineComponent(AttackComponent());
		man->defineComponent(AugmentComponent());
		man->defineComponent(MonsterComponent());
		man->defineComponent(MonsterDebuffComponent());
		man->defineComponent(EngineComponent());
		return man;
	}

	void engineInit()
	{
		eventGameReset().dispatch();
	}

	void gameReset()
	{
		gameTime = 0;
		gameSpeed = 1;
		gameRunning = false;
		gameEntities()->destroy();

		playerCursorPosition = vec3::Nan();
		playerCursorTile = m;
		playerHealth = 100;
		playerMoney = 500;
		playerBuildingSelection = 0;
	}

	bool gameUpdate()
	{
		gameTime++;
		if (playerHealth <= 0)
		{
			setScreenGameOver();
			return true;
		}
		return false;
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> gameResetListener;
		EventListener<bool()> gameUpdateListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize, 200);
			engineInitListener.bind<&engineInit>();
			gameResetListener.attach(eventGameReset(), -100);
			gameResetListener.bind<&gameReset>();
			gameUpdateListener.attach(eventGameUpdate(), -500);
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
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
uint32 gameSpeed = 1;
bool gameRunning = false;

vec3 playerCursorPosition;
uint32 playerCursorTile;
sint32 playerHealth;
sint32 playerMoney;
uint32 playerBuildingSelection;
bool playerPanning = false;

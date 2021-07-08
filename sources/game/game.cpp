#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

vec3 MovementComponent::position() const
{
	const vec3 ca = globalGrid->center(tileStart);
	const vec3 cb = globalGrid->center(tileEnd);
	const real fac = saturate(real(gameTime - (sint64)timeStart) / real(timeEnd - (sint64)timeStart));
	return interpolate(ca, cb, fac);
}

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
		man->defineComponent(ManaStorageComponent());
		man->defineComponent(ManaDistributorComponent());
		man->defineComponent(ManaReceiverComponent());
		man->defineComponent(ManaGeneratorComponent());
		man->defineComponent(AttackComponent());
		man->defineComponent(AugmentComponent());
		man->defineComponent(MonsterComponent());
		man->defineComponent(EngineComponent());
		return man;
	}

	void engineInit()
	{
		eventGameReset().dispatch();
	}

	void engineUpdate()
	{
		if (gameRunning)
			gameTime++;
	}

	void gameReset()
	{
		gameTime = 0;
		gameRunning = false;
		gameEntities()->destroy();

		playerCursorPosition = vec3::Nan();
		playerCursorTile = m;
		playerHealth = 100;
		playerMoney = 100;
		playerBuildingSelection = 0;
	}

	struct Callbacks
	{
		EventListener<void()> engineInitListener;
		EventListener<void()> engineUpdateListener;
		EventListener<void()> gameResetListener;

		Callbacks()
		{
			engineInitListener.attach(controlThread().initialize, 200);
			engineInitListener.bind<&engineInit>();
			engineUpdateListener.attach(controlThread().update, -500);
			engineUpdateListener.bind<&engineUpdate>();
			gameResetListener.attach(eventGameReset(), -100);
			gameResetListener.bind<&gameReset>();
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

uint32 gameTime = 0;
bool gameRunning = false;

vec3 playerCursorPosition;
uint32 playerCursorTile;
sint32 playerHealth;
uint32 playerMoney;
uint32 playerBuildingSelection;
bool playerPanning = false;

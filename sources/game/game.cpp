#include <cage-engine/engine.h>

#include "../game.h"

namespace
{
	Holder<EntityManager> initializeManager()
	{
		Holder<EntityManager> man = newEntityManager();
		man->defineComponent(PositionComponent());
		man->defineComponent(MovementComponent());
		man->defineComponent(WallComponent());
		man->defineComponent(MonsterComponent());
		man->defineComponent(EngineComponent());
		return man;
	}

	uint32 time = 0;

	void engineUpdate()
	{
		time++;
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

EntityManager *gameEntities()
{
	static Holder<EntityManager> man = initializeManager();
	return +man;
}

uint32 gameTime()
{
	return time;
}

vec3 playerCursorPosition = vec3::Nan();
uint32 playerCursorTile = m;
sint32 playerHealth = 100;
uint32 playerMoney = 100;

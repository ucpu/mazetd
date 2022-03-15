#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-core/profiling.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	Holder<SpatialStructure> monstersData = newSpatialStructure({});
	Holder<SpatialStructure> structsData = newSpatialStructure({});
	Holder<SpatialQuery> monstersQuery = newSpatialQuery(monstersData.share());
	Holder<SpatialQuery> structsQuery = newSpatialQuery(structsData.share());

	void gameReset()
	{
		monstersData->clear();
		structsData->clear();
	}

	void gameUpdate()
	{
		monstersData->clear();
		CAGE_ASSERT(globalGrid);

		entitiesVisitor([&](Entity *e, const MovementComponent &mv, const MonsterComponent &) {
			monstersData->update(e->name(), mv.position());
		}, gameEntities(), false);

		monstersData->rebuild();
	}

	struct Callbacks
	{
		EventListener<void()> gameResetListener;
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameResetListener.attach(eventGameReset());
			gameResetListener.bind<&gameReset>();
			gameUpdateListener.attach(eventGameUpdate(), 30);
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

SpatialQuery *spatialMonsters()
{
	return +monstersQuery;
}

SpatialQuery *spatialStructures()
{
	return +structsQuery;
}

void spatialUpdateStructures()
{
	ProfilingScope profiling("spatial update structures", "spatial");

	structsData->clear();
	CAGE_ASSERT(globalGrid);

	entitiesVisitor([&](Entity *e, const PositionComponent &pos, const BuildingComponent &) {
		structsData->update(e->name(), globalGrid->center(pos.tile) * Vec3(1, 0, 1));
	}, gameEntities(), false);

	entitiesVisitor([&](Entity *e, const PositionComponent &pos, const TrapComponent &) {
		structsData->update(e->name(), globalGrid->center(pos.tile) * Vec3(1, 0, 1));
	}, gameEntities(), false);

	structsData->rebuild();
}

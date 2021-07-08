#include <cage-core/entitiesVisitor.h>
#include <cage-core/spatialStructure.h>
#include <cage-engine/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	Holder<SpatialStructure> monstersData = newSpatialStructure({});
	Holder<SpatialStructure> structsData = newSpatialStructure({});
	Holder<SpatialQuery> monstersQuery = newSpatialQuery(monstersData.share());
	Holder<SpatialQuery> structsQuery = newSpatialQuery(structsData.share());

	void engineUpdate()
	{
		monstersData->clear();
		structsData->clear();
		if (!globalGrid)
			return;

		entitiesVisitor(gameEntities(), [&](Entity *e, const MovementComponent &mv, const MonsterComponent &) {
			monstersData->update(e->name(), mv.position());
		});

		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, const BuildingComponent &) {
			structsData->update(e->name(), globalGrid->center(pos.tile));
		});

		entitiesVisitor(gameEntities(), [&](Entity *e, const PositionComponent &pos, const TrapComponent &) {
			structsData->update(e->name(), globalGrid->center(pos.tile));
		});

		monstersData->rebuild();
		structsData->rebuild();
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, 30);
			engineUpdateListener.bind<&engineUpdate>();
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

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

	const auto gameResetListener = eventGameReset().listen([]() {
		monstersData->clear();
		structsData->clear();
	});

	const auto gameUpdateListener = eventGameUpdate().listen([]() {
		monstersData->clear();
		CAGE_ASSERT(globalGrid);

		entitiesVisitor([&](Entity *e, const MovementComponent &mv, const MonsterComponent &) {
			monstersData->update(e->name(), mv.position() * Vec3(1, 0, 1));
		}, gameEntities(), false);

		monstersData->rebuild();
	}, 30);
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
	ProfilingScope profiling("spatial update structures");

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

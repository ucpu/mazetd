#include <cage-core/entitiesVisitor.h>
#include <cage-core/camera.h>
#include <cage-core/geometry.h>
#include <cage-core/collider.h>
#include <cage-core/hashString.h>
#include <cage-engine/window.h>
#include <cage-engine/scene.h>
#include <cage-simple/engine.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	Entity *cursorMarker = nullptr;
	Entity *towersMarkers[500] = {};

	template<uint32 Object>
	void markPos(Entity *&e, uint32 position, Real scale = 1, Real offset = 0)
	{
		if (position == m && e)
		{
			e->destroy();
			e = nullptr;
		}
		else if (position != m)
		{
			if (!e)
				e = engineEntities()->createAnonymous();
			e->value<RenderComponent>().object = Object;
			e->value<TransformComponent>().position = globalGrid->center(position) + Vec3(0, offset, 0);
			e->value<TransformComponent>().scale = scale;
		}
	}

	template<uint32 Object>
	void markEnt(Entity *&e, Entity *ent, Real scale = 1, Real offset = 0)
	{
		markPos<Object>(e, ent ? ent->value<PositionComponent>().tile : m, scale, offset);
	}

	void updateCursorMarker()
	{
		markPos<HashString("mazetd/misc/cursor.obj")>(cursorMarker, playerCursorTile != m && none(globalGrid->flags[playerCursorTile] & TileFlags::Invalid) ? playerCursorTile : m);
	}

	bool affected(Entity *e, const AttackComponent &mods)
	{
		for (const auto &it : mods.effectors)
			if (it == e)
				return true;
		return false;
	}

	void updateTowerMods()
	{
		Entity *currEnt = nullptr;
		if (playerCursorTile != m)
		{
			entitiesVisitor([&](Entity *e, const PositionComponent &po) {
				if (po.tile == playerCursorTile)
					currEnt = e;
			}, gameEntities(), false);
		}

		uint32 tm = 0;

		if (currEnt)
		{
			{ // mods
				const AttackComponent *mods = currEnt->has<AttackComponent>() ? &currEnt->value<AttackComponent>() : nullptr;
				for (uint32 i = 0; i < 3; i++)
				{
					Entity *ent = mods ? mods->effectors[i] : nullptr;
					if (ent)
						markEnt<HashString("mazetd/misc/modMark.obj")>(towersMarkers[tm++], ent);
				}
			}

			// towers
			entitiesVisitor([&](const PositionComponent &po, const AttackComponent &atc) {
				if (affected(currEnt, atc))
					markPos<HashString("mazetd/misc/modMark.obj")>(towersMarkers[tm++], po.tile);
			}, gameEntities(), false);

			// attack range
			if (currEnt->has<DamageComponent>())
			{
				Real r = currEnt->value<DamageComponent>().firingRange;
				CAGE_ASSERT(currEnt->has<AttackComponent>());
				if (currEnt->value<AttackComponent>().enhancement == EnhancementTypeEnum::FiringRange)
					r += 4; // keep in sync with actual attacks
				markEnt<HashString("mazetd/misc/attackRangeMark.obj")>(towersMarkers[tm++], currEnt, r, 1);
			}

			// mana collector range
			if (currEnt->has<ManaCollectorComponent>())
				markEnt<HashString("mazetd/misc/manaRangeMark.obj")>(towersMarkers[tm++], currEnt, currEnt->value<ManaCollectorComponent>().range, 1);

			// mana distributors
			if (currEnt->has<ManaReceiverComponent>())
			{
				const Vec3 p = currEnt->value<PositionComponent>().position() * Vec3(1, 0, 1);
				entitiesVisitor([&](const PositionComponent &po, const ManaDistributorComponent &man) {
					if (distanceSquared(p, po.position() * Vec3(1, 0, 1)) < sqr(man.range))
						markPos<HashString("mazetd/misc/manaInteractionMark.obj")>(towersMarkers[tm++], po.tile);
				}, gameEntities(), false);
			}

			// mana receivers
			if (currEnt->has<ManaDistributorComponent>())
			{
				const Real r2 = sqr(currEnt->value<ManaDistributorComponent>().range);
				const Vec3 p = currEnt->value<PositionComponent>().position() * Vec3(1, 0, 1);
				entitiesVisitor([&](const PositionComponent &po, const ManaReceiverComponent &) {
					if (distanceSquared(p, po.position() * Vec3(1, 0, 1)) < r2)
						markPos<HashString("mazetd/misc/manaInteractionMark.obj")>(towersMarkers[tm++], po.tile);
				}, gameEntities(), false);
			}
		}

		while (tm < sizeof(towersMarkers) / sizeof(towersMarkers[0]))
			markPos<0>(towersMarkers[tm++], m);
	}

	void engineUpdate()
	{
		playerCursorPosition = Vec3::Nan();
		playerCursorTile = m;
		const Vec2i res = engineWindow()->resolution();
		if (gameReady && !playerPanning && engineWindow()->isFocused() && res[0] > 0 && res[1] > 0)
		{
			CAGE_ASSERT(globalGrid);
			const Vec2 cur = engineWindow()->mousePosition();
			Entity *c = engineEntities()->component<CameraComponent>()->entities()[0];
			const CameraComponent &a = c->value<CameraComponent>();
			const Mat4 view = Mat4(inverse(c->value<TransformComponent>()));
			const Mat4 proj = [&]() {
				if (a.cameraType == CameraTypeEnum::Perspective)
					return perspectiveProjection(a.camera.perspectiveFov, Real(res[0]) / Real(res[1]), a.near, a.far);
				const Vec2 &os = a.camera.orthographicSize;
				return orthographicProjection(-os[0], os[0], -os[1], os[1], a.near, a.far);
			}();
			const Mat4 inv = inverse(proj * view);
			const Vec2 cp = (Vec2(cur) / Vec2(res) * 2 - 1) * Vec2(1, -1);
			const Vec4 pn = inv * Vec4(cp, -1, 1);
			const Vec4 pf = inv * Vec4(cp, 1, 1);
			const Vec3 near = Vec3(pn) / pn[3];
			const Vec3 far = Vec3(pf) / pf[3];
			const Line line = makeSegment(near, far);
			playerCursorPosition = intersection(line, +globalCollider, Transform());
			if (playerCursorPosition.valid())
				playerCursorTile = globalGrid->index(playerCursorPosition);
		}
		updateCursorMarker();
		updateTowerMods();
	}

	struct Callbacks
	{
		EventListener<void()> engineUpdateListener;

		Callbacks()
		{
			engineUpdateListener.attach(controlThread().update, -100);
			engineUpdateListener.bind<&engineUpdate>();
		}
	} callbacksInstance;
}

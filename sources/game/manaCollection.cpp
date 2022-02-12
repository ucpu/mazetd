#include <cage-core/entitiesVisitor.h>
#include <cage-core/profiling.h>

#include "../game.h"
#include "../grid.h"

namespace
{
	void placeNewMana()
	{
		if ((gameTime % 3) == 0)
			return;

		ProfilingScope profiling("place mana", "mana");

		// populates up to 20 tiles per second -> up to 100 mana per second
		const uint32 totalTiles = globalGrid->resolution[0] * globalGrid->resolution[1];
		uint32 occupied = 0;
		while (true)
		{
			const uint32 tile = randomRange(0u, totalTiles);
			TileFlags &f = globalGrid->flags[tile];
			if (any(f & TileFlags::Invalid))
				continue;
			if (any(f & TileFlags::Mana))
			{
				if (++occupied == 3) // several attempts -> player needs to cover only portion of the map to collect most of the mana
					break;
				else
					continue;
			}
			f |= TileFlags::Mana;
			break;
		}
	}

	TileFlags requiredFlags(ManaCollectorTypeEnum type)
	{
		switch (type)
		{
		case ManaCollectorTypeEnum::Water: return TileFlags::Water;
		case ManaCollectorTypeEnum::Sun: return TileFlags::Sun;
		case ManaCollectorTypeEnum::Wind: return TileFlags::Wind;
		case ManaCollectorTypeEnum::Snow: return TileFlags::Snow;
		default: CAGE_THROW_CRITICAL(Exception, "invalid ManaCollectorTypeEnum");
		}
	}

	void gameUpdate()
	{
		placeNewMana();

		ProfilingScope profiling("collect mana", "mana");

		entitiesVisitor([](Entity *e, const PositionComponent &pos, const ManaCollectorComponent &col, ManaStorageComponent &stor) {
			if (stor.mana + col.collectAmount > stor.capacity)
				return;

			const Vec2i mp2 = globalGrid->position(pos.tile);
			const Vec3 mp3 = globalGrid->center(pos.tile);
			const sint32 xa = mp2[0] - col.range.value - 0.5;
			const sint32 xb = mp2[0] + col.range.value + 0.5;
			const sint32 ya = mp2[1] - col.range.value - 0.5;
			const sint32 yb = mp2[1] + col.range.value + 0.5;
			for (sint32 y = ya; y <= yb; y++)
			{
				for (sint32 x = xa; x <= xb; x++)
				{
					const uint32 t = globalGrid->index(Vec2i(x, y));
					if (t == m)
						continue;
					TileFlags &f = globalGrid->flags[t];
					if (none(f & TileFlags::Mana))
						continue;
					if (none(f & requiredFlags(col.type)))
						continue;
					const Vec3 p = globalGrid->center(t);
					if (distanceSquared(mp3, p) > sqr(col.range))
						continue;

					f &= ~TileFlags::Mana;
					stor.mana += col.collectAmount;

					{
						EffectConfig cfg;
						cfg.pos1 = p;
						cfg.pos2 = mp3 + Vec3(0, e->value<PivotComponent>().elevation, 0);
						cfg.type = DamageTypeEnum::Magic;
						renderEffect(cfg);
					}

					if (stor.mana + col.collectAmount > stor.capacity)
						return;
				}
			}
		}, gameEntities(), false);
	}

	struct Callbacks
	{
		EventListener<void()> gameUpdateListener;

		Callbacks()
		{
			gameUpdateListener.attach(eventGameUpdate());
			gameUpdateListener.bind<&gameUpdate>();
		}
	} callbacksInstance;
}

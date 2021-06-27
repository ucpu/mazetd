#include "grid.h"

struct Paths : private Immovable
{
	Holder<const Grid> grid;
	uint32 targetTile = m;

	Holder<PointerRange<uint32>> paths; // -1 = no path
	Holder<PointerRange<uint32>> distances; // 16.16 format

	void update();
};

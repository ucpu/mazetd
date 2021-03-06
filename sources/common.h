#ifndef header_common
#define header_common

#include <cage-core/math.h>

using namespace cage;

CAGE_FORCE_INLINE constexpr sint16 rtos16(Real v)
{
	return numeric_cast<sint16>(v * 255);
}

CAGE_FORCE_INLINE constexpr sint32 rtos32(Real v)
{
	return numeric_cast<sint32>(v * 255);
}

CAGE_FORCE_INLINE constexpr Real stor(sint32 v)
{
	constexpr Real n = 1.0 / 255;
	return v * n;
}

uint32 bitCount(uint32 v);

#endif // !header_common

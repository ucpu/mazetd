#ifndef header_normalizedReal
#define header_normalizedReal

#include <cage-core/math.h>

using namespace cage;

CAGE_FORCE_INLINE constexpr sint16 rtos16(real v)
{
	return numeric_cast<sint16>(v * 255);
}

CAGE_FORCE_INLINE constexpr sint32 rtos32(real v)
{
	return numeric_cast<sint32>(v * 255);
}

CAGE_FORCE_INLINE constexpr real stor(sint32 v)
{
	constexpr real n = 1.0 / 255;
	return v * n;
}

#endif // !header_normalizedReal

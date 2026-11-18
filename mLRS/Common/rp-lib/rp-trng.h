//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// RP True Random Number Generator
//*******************************************************
#ifndef RP_TRNG_H
#define RP_TRNG_H

#include <pico/rand.h>

inline void trng_init(void) {}

inline uint32_t trng_get32(void)
{
    return get_rand_32();
}

#endif // RP_TRNG_H

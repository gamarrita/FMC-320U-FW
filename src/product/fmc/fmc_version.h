/**
 * @file    fmc_version.h
 * @brief   Provisional FMC firmware version metadata.
 *
 * Phase 6A uses a deliberately unpublished dummy version. The numeric
 * components are still constrained to the two-digit LCD representation so an
 * out-of-range component fails at compile time instead of being truncated.
 */

#ifndef FMC_VERSION_H
#define FMC_VERSION_H

#define FMC_VERSION_MAJOR       0
#define FMC_VERSION_MINOR       1
#define FMC_VERSION_PATCH       0
#define FMC_VERSION_BETA        0
#define FMC_VERSION_IS_DUMMY    1

#if (FMC_VERSION_MAJOR < 0) || (FMC_VERSION_MAJOR > 99)
#error "FMC_VERSION_MAJOR must fit the LCD MM field."
#endif

#if (FMC_VERSION_MINOR < 0) || (FMC_VERSION_MINOR > 99)
#error "FMC_VERSION_MINOR must fit the LCD mm field."
#endif

#if (FMC_VERSION_PATCH < 0) || (FMC_VERSION_PATCH > 99)
#error "FMC_VERSION_PATCH must fit the LCD pp field."
#endif

#if (FMC_VERSION_BETA < 0) || (FMC_VERSION_BETA > 9)
#error "FMC_VERSION_BETA must fit the LCD Bn field."
#endif

#if (FMC_VERSION_IS_DUMMY != 1)
#if (FMC_VERSION_BETA == 0)
#define FMC_VERSION_IS_STABLE    1
#else
#define FMC_VERSION_IS_STABLE    0
#endif
#endif

#endif /* FMC_VERSION_H */

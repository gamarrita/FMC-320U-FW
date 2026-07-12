/**
 * @file fm_factory.h
 * @brief Default factory values and helpers for the FMC-320U.
 */

#ifndef FM_FACTORY_H_
#define FM_FACTORY_H_

#include "fm_fmc.h"

fm_fmc_totalizer_t FM_FACTORY_TotalizerGet(sensors_list_t sel);
const char *FM_FACTORY_FirmwareVersionGet(void);
const char *FM_FACTORY_ReleaseGet(void);

#endif // FM_FACTORY_H_


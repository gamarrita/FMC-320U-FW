#ifndef FM_LOG_POLICY_H
#define FM_LOG_POLICY_H

#include <stdint.h>
#include <stdbool.h>
#include "fm_fmc.h"


// --- Macros por defecto ---
#define FM_LOG_POLICY_MAX_CREDITS      (10u)
#define FM_LOG_POLICY_VARIATION_PPM    (20000u)      // 2 %

// --- Tipos públicos ---

typedef struct {
    uint8_t   max_credits;     // default: 10
    uint32_t  heartbeat_sec;   // default: 3600 (60 min)
    uint32_t  variation_ppm;   // default: 20000 (2 %)
} fm_log_policy_cfg_t;


// --- API pública ---

uint32_t FM_LOG_POLICY_Step(fmx_ack_t ack);


#endif // FM_LOG_POLICY_H

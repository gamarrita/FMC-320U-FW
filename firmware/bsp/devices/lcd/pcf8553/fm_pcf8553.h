/**
 * @file    fm_pcf8553.h
 * @brief   Public backend contract for the redesigned PCF8553 path.
 *
 * This header defines the intended controller-facing contract for the new LCD
 * backend. It is a contract artifact, not the final implementation.
 *
 * Current backend intent:
 * - own controller-specific startup, reset, and resume behavior
 * - own controller-specific RAM write behavior
 * - expose explicit write-range support for partial LCD flushes
 * - stay independent from LCD glass semantics and field naming
 *
 * Current non-goals:
 * - owning the desired LCD state
 * - owning the LCD RAM image used by higher layers
 * - exposing glass-level rows, indicators, or alphanumeric semantics
 * - defining public blink semantics for the LCD stack
 */

#ifndef FM_PCF8553_H_
#define FM_PCF8553_H_

#include <stdint.h>

/* =========================== Public Macros ============================= */
#define FM_PCF8553_RAM_SIZE    20U

/* =========================== Public Types ============================== */
/**
 * @brief Public result codes for the PCF8553 backend API.
 *
 * Contract rule:
 * - this backend owns controller-facing behavior and therefore reports
 *   argument, range, state, and I/O failures explicitly
 * - public LCD semantics should be handled by upper layers, not encoded here
 *
 * Status code intent:
 * - FM_PCF8553_OK: operation completed successfully
 * - FM_PCF8553_EINVAL: invalid argument value or invalid argument combination
 * - FM_PCF8553_ERANGE: argument or requested range exceeds backend limits
 * - FM_PCF8553_ESTATE: backend state does not allow the requested operation
 * - FM_PCF8553_EIO: controller reset, transport, or communication step failed
 */
typedef enum
{
    FM_PCF8553_OK = 0,
    FM_PCF8553_EINVAL,
    FM_PCF8553_ERANGE,
    FM_PCF8553_ESTATE,
    FM_PCF8553_EIO
} fm_pcf8553_status_t;

/* =========================== Public API ================================ */
/**
 * @brief Initialize the PCF8553 backend for first use.
 *
 * This initialization path is expected to own controller-specific startup
 * requirements such as reset and any datasheet-required power-on handling.
 *
 * This function does not own the desired LCD content model.
 *
 * @return FM_PCF8553_OK on success.
 * @return FM_PCF8553_EIO on transport or controller communication failure.
 */
fm_pcf8553_status_t FM_PCF8553_Init(void);

/**
 * @brief Apply a controller reset sequence.
 *
 * This function resets the controller to a known backend-owned state.
 * Any controller-side RAM contents should be considered lost or undefined after
 * reset unless later backend behavior guarantees otherwise.
 *
 * @return FM_PCF8553_OK on success.
 * @return FM_PCF8553_EIO on reset control or communication failure.
 */
fm_pcf8553_status_t FM_PCF8553_Reset(void);

/**
 * @brief Reapply the controller operational configuration after wake or reset.
 *
 * This function is intended to restore backend-owned configuration without
 * defining higher-layer LCD content policy.
 *
 * Rewriting visible LCD RAM remains an upper-layer decision.
 *
 * @return FM_PCF8553_OK on success.
 * @return FM_PCF8553_ESTATE when the backend has not been initialized.
 * @return FM_PCF8553_EIO on transport or controller communication failure.
 */
fm_pcf8553_status_t FM_PCF8553_Resume(void);

/**
 * @brief Write a contiguous RAM range to the PCF8553 display RAM.
 *
 * The RAM offset is relative to the PCF8553 display RAM space, not to the
 * raw command-register address space.
 *
 * The backend is expected to translate this offset into the appropriate device
 * write sequence.
 *
 * @param[in] p_ram_offset Zero-based offset into PCF8553 display RAM.
 * @param[in] p_data Pointer to the RAM bytes to write.
 * @param[in] p_len Number of bytes to write.
 *
 * @return FM_PCF8553_OK on success.
 * @return FM_PCF8553_EINVAL when `p_data` is NULL.
 * @return FM_PCF8553_ERANGE when the requested RAM range exceeds
 *         `FM_PCF8553_RAM_SIZE`.
 * @return FM_PCF8553_ESTATE when the backend has not been initialized.
 * @return FM_PCF8553_EIO on transport or controller communication failure.
 */
fm_pcf8553_status_t FM_PCF8553_WriteRam(uint8_t p_ram_offset,
                                        const uint8_t *p_data,
                                        uint8_t p_len);

#endif /* FM_PCF8553_H_ */

/*** end of file ***/

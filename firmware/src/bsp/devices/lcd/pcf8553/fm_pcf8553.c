/**
 * @file    fm_pcf8553.c
 * @brief   Redesigned PCF8553 backend implementation.
 *
 * This module implements the new backend contract without depending on the
 * frozen legacy backend path.
 */

#include "fm_pcf8553.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "fm_port_pcf8553_ctrl.h"
#include "fm_port_spi1.h"

/* =========================== Private Types ============================== */
typedef struct
{
    bool initialized;
    bool configuration_applied;
} fm_pcf8553_state_t;

typedef union
{
    uint8_t data;
    struct
    {
        uint8_t address : 5;
        uint8_t not_used : 2;
        uint8_t read_write : 1;
    } bits;
} fm_pcf8553_register_address_t;

/* =========================== Private Macros ============================= */
#define FM_PCF8553_SPI_TIMEOUT_MS             5U
#define FM_PCF8553_POST_RESET_DELAY_MS        5U
#define FM_PCF8553_DATA_ADDRESS               0x04U
#define FM_PCF8553_DEVICE_CTRL_ADDRESS        0x01U
#define FM_PCF8553_DISPLAY_CTRL_1_ADDRESS     0x02U
#define FM_PCF8553_DISPLAY_CTRL_2_ADDRESS     0x03U
#define FM_PCF8553_CONFIGURATION_SIZE         3U
#define FM_PCF8553_WRITE_DATA                 0U

/* =========================== Private Data =============================== */
/*
 * Board note:
 * - PORE and IFS are strapped to VDD on the current hardware.
 * - This backend therefore owns CE/reset sequencing and the runtime register
 *   configuration sequence, not those hardware strap decisions.
 */
static fm_pcf8553_state_t g_fm_pcf8553_state;

/*
 * Runtime controller configuration owned by this backend:
 * - Device_ctrl      = 0x00: no clock output, external clock mode not used
 * - Display_ctrl_1   = 0x01: display enabled, 1/3 bias, 1:4 mux, boost off
 * - Display_ctrl_2   = 0x00: line inversion, blink off
 */
static const uint8_t g_fm_pcf8553_configuration[FM_PCF8553_CONFIGURATION_SIZE] =
{
    0x00U,
    0x01U,
    0x00U
};

/* =========================== Private Prototypes ========================= */
static fm_pcf8553_status_t fm_pcf8553_apply_configuration_(void);
static uint8_t fm_pcf8553_build_write_address_(uint8_t p_address);
static fm_pcf8553_status_t fm_pcf8553_validate_ram_range_(uint8_t p_ram_offset,
                                                          uint8_t p_len);
static bool fm_pcf8553_write_transaction_(const uint8_t *p_data, uint16_t p_len);
static fm_pcf8553_status_t fm_pcf8553_write_sequence_(uint8_t p_start_address,
                                                      const uint8_t *p_data,
                                                      uint16_t p_len);

/* =========================== Private Bodies ============================= */
static fm_pcf8553_status_t fm_pcf8553_apply_configuration_(void)
{
    fm_pcf8553_status_t status;

    status = fm_pcf8553_write_sequence_(FM_PCF8553_DEVICE_CTRL_ADDRESS,
                                        g_fm_pcf8553_configuration,
                                        FM_PCF8553_CONFIGURATION_SIZE);

    if (status != FM_PCF8553_OK)
    {
        g_fm_pcf8553_state.configuration_applied = false;
        return status;
    }

    g_fm_pcf8553_state.configuration_applied = true;

    return FM_PCF8553_OK;
}

static uint8_t fm_pcf8553_build_write_address_(uint8_t p_address)
{
    fm_pcf8553_register_address_t register_address = { 0 };

    register_address.bits.address = p_address;
    register_address.bits.not_used = 0U;
    register_address.bits.read_write = FM_PCF8553_WRITE_DATA;

    return register_address.data;
}

static fm_pcf8553_status_t fm_pcf8553_validate_ram_range_(uint8_t p_ram_offset,
                                                          uint8_t p_len)
{
    if (p_len == 0U)
    {
        return FM_PCF8553_EINVAL;
    }

    if (((uint16_t)p_ram_offset + (uint16_t)p_len) > FM_PCF8553_RAM_SIZE)
    {
        return FM_PCF8553_ERANGE;
    }

    return FM_PCF8553_OK;
}

static bool fm_pcf8553_write_transaction_(const uint8_t *p_data, uint16_t p_len)
{
    bool success;

    if ((p_data == NULL) || (p_len == 0U))
    {
        return false;
    }

    FM_PORT_Pcf8553Ctrl_Enable();
    success = FM_PORT_Spi1_Write(p_data, p_len, FM_PCF8553_SPI_TIMEOUT_MS);
    FM_PORT_Pcf8553Ctrl_Disable();

    return success;
}

static fm_pcf8553_status_t fm_pcf8553_write_sequence_(uint8_t p_start_address,
                                                      const uint8_t *p_data,
                                                      uint16_t p_len)
{
    uint8_t frame[FM_PCF8553_RAM_SIZE + 1U];

    if (p_data == NULL)
    {
        return FM_PCF8553_EINVAL;
    }

    if ((p_len == 0U) || (p_len > FM_PCF8553_RAM_SIZE))
    {
        return FM_PCF8553_ERANGE;
    }

    frame[0] = fm_pcf8553_build_write_address_(p_start_address);
    (void) memcpy(&frame[1], p_data, p_len);

    if (!fm_pcf8553_write_transaction_(frame, (uint16_t)(p_len + 1U)))
    {
        return FM_PCF8553_EIO;
    }

    return FM_PCF8553_OK;
}

/* =========================== Public Bodies ============================== */
fm_pcf8553_status_t FM_PCF8553_Init(void)
{
    fm_pcf8553_status_t status;

    g_fm_pcf8553_state.initialized = false;
    g_fm_pcf8553_state.configuration_applied = false;

    status = FM_PCF8553_Reset();

    if (status != FM_PCF8553_OK)
    {
        return status;
    }

    status = fm_pcf8553_apply_configuration_();

    if (status != FM_PCF8553_OK)
    {
        return status;
    }

    g_fm_pcf8553_state.initialized = true;

    return FM_PCF8553_OK;
}

fm_pcf8553_status_t FM_PCF8553_Reset(void)
{
    bool was_initialized;

    was_initialized = g_fm_pcf8553_state.initialized;
    FM_PORT_Pcf8553Ctrl_Reset();
    HAL_Delay(FM_PCF8553_POST_RESET_DELAY_MS);
    g_fm_pcf8553_state.initialized = was_initialized;
    g_fm_pcf8553_state.configuration_applied = false;

    return FM_PCF8553_OK;
}

fm_pcf8553_status_t FM_PCF8553_Resume(void)
{
    if (!g_fm_pcf8553_state.initialized)
    {
        return FM_PCF8553_ESTATE;
    }

    return fm_pcf8553_apply_configuration_();
}

fm_pcf8553_status_t FM_PCF8553_WriteRam(uint8_t p_ram_offset,
                                        const uint8_t *p_data,
                                        uint8_t p_len)
{
    fm_pcf8553_status_t status;

    if (p_data == NULL)
    {
        return FM_PCF8553_EINVAL;
    }

    status = fm_pcf8553_validate_ram_range_(p_ram_offset, p_len);

    if (status != FM_PCF8553_OK)
    {
        return status;
    }

    if ((!g_fm_pcf8553_state.initialized) ||
        (!g_fm_pcf8553_state.configuration_applied))
    {
        return FM_PCF8553_ESTATE;
    }

    return fm_pcf8553_write_sequence_((uint8_t)(FM_PCF8553_DATA_ADDRESS + p_ram_offset),
                                      p_data,
                                      p_len);
}

/*** end of file ***/

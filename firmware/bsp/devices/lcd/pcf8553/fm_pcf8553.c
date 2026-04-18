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

typedef union
{
    uint8_t reg_data;
    struct
    {
        uint8_t clock_output : 1;
        uint8_t internal_oscillator : 1;
        uint8_t frame_frequency : 2;
        uint8_t reserved : 4;
    } reg_bits;
} fm_pcf8553_device_ctrl_t;

typedef union
{
    uint8_t reg_data;
    struct
    {
        uint8_t display_enabled : 1;
        uint8_t bias_mode : 1;
        uint8_t mux : 2;
        uint8_t boost : 1;
        uint8_t reserved : 3;
    } reg_bits;
} fm_pcf8553_display_ctrl_1_t;

typedef union
{
    uint8_t reg_data;
    struct
    {
        uint8_t inversion : 1;
        uint8_t blink : 2;
        uint8_t reserved : 5;
    } reg_bits;
} fm_pcf8553_display_ctrl_2_t;

/* =========================== Private Macros ============================= */
#define FM_PCF8553_SPI_TIMEOUT_MS             5U
#define FM_PCF8553_POST_RESET_DELAY_MS        5U
#define FM_PCF8553_DATA_ADDRESS               0x04U
#define FM_PCF8553_DEVICE_CTRL_ADDRESS        0x01U
#define FM_PCF8553_DISPLAY_CTRL_1_ADDRESS     0x02U
#define FM_PCF8553_DISPLAY_CTRL_2_ADDRESS     0x03U
#define FM_PCF8553_WRITE_DATA                 0U

/* =========================== Private Data =============================== */
static bool g_fm_pcf8553_initialized_;

static fm_pcf8553_device_ctrl_t g_fm_pcf8553_device_ctrl_ =
{
    .reg_bits.clock_output = 0U,
    .reg_bits.internal_oscillator = 0U,
    .reg_bits.frame_frequency = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_display_ctrl_1_t g_fm_pcf8553_display_ctrl_1_ =
{
    .reg_bits.display_enabled = 1U,
    .reg_bits.bias_mode = 0U,
    .reg_bits.mux = 0U,
    .reg_bits.boost = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_display_ctrl_2_t g_fm_pcf8553_display_ctrl_2_ =
{
    .reg_bits.inversion = 0U,
    .reg_bits.blink = 0U,
    .reg_bits.reserved = 0U
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
    uint8_t init_registers[] =
    {
        g_fm_pcf8553_device_ctrl_.reg_data,
        g_fm_pcf8553_display_ctrl_1_.reg_data,
        g_fm_pcf8553_display_ctrl_2_.reg_data
    };

    return fm_pcf8553_write_sequence_(FM_PCF8553_DEVICE_CTRL_ADDRESS,
                                      init_registers,
                                      (uint16_t) sizeof(init_registers));
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

    g_fm_pcf8553_initialized_ = false;

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

    g_fm_pcf8553_initialized_ = true;

    return FM_PCF8553_OK;
}

fm_pcf8553_status_t FM_PCF8553_Reset(void)
{
    FM_PORT_Pcf8553Ctrl_Reset();
    HAL_Delay(FM_PCF8553_POST_RESET_DELAY_MS);

    return FM_PCF8553_OK;
}

fm_pcf8553_status_t FM_PCF8553_Resume(void)
{
    if (!g_fm_pcf8553_initialized_)
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

    if (!g_fm_pcf8553_initialized_)
    {
        return FM_PCF8553_ESTATE;
    }

    return fm_pcf8553_write_sequence_((uint8_t)(FM_PCF8553_DATA_ADDRESS + p_ram_offset),
                                      p_data,
                                      p_len);
}

/*** end of file ***/

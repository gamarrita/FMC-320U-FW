/**
 * @file    fm_pcf8553_legacy.c
 * @brief   Frozen legacy PCF8553 backend implementation.
 *
 * This file preserves the historical controller path as a working backup while
 * the redesigned backend is implemented independently in `fm_pcf8553.c`.
 */

#include "fm_pcf8553_legacy.h"

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
} fm_pcf8553_legacy_register_address_t;

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
} fm_pcf8553_legacy_device_ctrl_t;

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
} fm_pcf8553_legacy_display_ctrl_1_t;

typedef union
{
    uint8_t reg_data;
    struct
    {
        uint8_t inversion : 1;
        uint8_t blink : 2;
        uint8_t reserved : 5;
    } reg_bits;
} fm_pcf8553_legacy_display_ctrl_2_t;

/* =========================== Private Macros ============================= */
#define FM_PCF8553_LEGACY_SPI_TIMEOUT_MS             5U
#define FM_PCF8553_LEGACY_POST_RESET_DELAY_MS        5U
#define FM_PCF8553_LEGACY_DATA_ADDRESS               0x04U
#define FM_PCF8553_LEGACY_DEVICE_CTRL_ADDRESS        0x01U
#define FM_PCF8553_LEGACY_DISPLAY_CTRL_1_ADDRESS     0x02U
#define FM_PCF8553_LEGACY_DISPLAY_CTRL_2_ADDRESS     0x03U
#define FM_PCF8553_LEGACY_WRITE_DATA                 0U

/* =========================== Private Data =============================== */
uint8_t g_fm_pcf8553_legacy_ram_map[FM_PCF8553_LEGACY_RAM_SIZE];

static fm_pcf8553_legacy_device_ctrl_t g_fm_pcf8553_legacy_device_ctrl_ =
{
    .reg_bits.clock_output = 0U,
    .reg_bits.internal_oscillator = 0U,
    .reg_bits.frame_frequency = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_legacy_display_ctrl_1_t g_fm_pcf8553_legacy_display_ctrl_1_ =
{
    .reg_bits.display_enabled = 1U,
    .reg_bits.bias_mode = 0U,
    .reg_bits.mux = 0U,
    .reg_bits.boost = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_legacy_display_ctrl_2_t g_fm_pcf8553_legacy_display_ctrl_2_ =
{
    .reg_bits.inversion = 0U,
    .reg_bits.blink = 0U,
    .reg_bits.reserved = 0U
};

/* =========================== Private Prototypes ========================= */
static uint8_t fm_pcf8553_legacy_build_write_address_(uint8_t p_address);
static bool fm_pcf8553_legacy_write_transaction_(const uint8_t *p_data, uint16_t p_len);
static void fm_pcf8553_legacy_write_sequence_(uint8_t p_start_address,
                                              const uint8_t *p_data,
                                              uint16_t p_len);

/* =========================== Private Bodies ============================= */
static uint8_t fm_pcf8553_legacy_build_write_address_(uint8_t p_address)
{
    fm_pcf8553_legacy_register_address_t register_address = { 0 };

    register_address.bits.address = p_address;
    register_address.bits.not_used = 0U;
    register_address.bits.read_write = FM_PCF8553_LEGACY_WRITE_DATA;

    return register_address.data;
}

static bool fm_pcf8553_legacy_write_transaction_(const uint8_t *p_data, uint16_t p_len)
{
    bool success;

    if ((p_data == NULL) || (p_len == 0U))
    {
        return false;
    }

    FM_PORT_Pcf8553Ctrl_Enable();
    success = FM_PORT_Spi1_Write(p_data, p_len, FM_PCF8553_LEGACY_SPI_TIMEOUT_MS);
    FM_PORT_Pcf8553Ctrl_Disable();

    return success;
}

static void fm_pcf8553_legacy_write_sequence_(uint8_t p_start_address,
                                              const uint8_t *p_data,
                                              uint16_t p_len)
{
    uint8_t frame[FM_PCF8553_LEGACY_RAM_SIZE + 1U];

    if ((p_data == NULL) || (p_len == 0U) || (p_len > FM_PCF8553_LEGACY_RAM_SIZE))
    {
        Error_Handler();
        return;
    }

    frame[0] = fm_pcf8553_legacy_build_write_address_(p_start_address);
    (void) memcpy(&frame[1], p_data, p_len);

    if (!fm_pcf8553_legacy_write_transaction_(frame, (uint16_t)(p_len + 1U)))
    {
        Error_Handler();
    }
}

/* =========================== Public Bodies ============================== */
void FM_PCF8553_LEGACY_ClearBuffer(void)
{
    (void) memset(g_fm_pcf8553_legacy_ram_map,
                  FM_PCF8553_LEGACY_SEGMENTS_OFF,
                  sizeof(g_fm_pcf8553_legacy_ram_map));
}

void FM_PCF8553_LEGACY_Init(void)
{
    uint8_t init_registers[] =
    {
        g_fm_pcf8553_legacy_device_ctrl_.reg_data,
        g_fm_pcf8553_legacy_display_ctrl_1_.reg_data,
        g_fm_pcf8553_legacy_display_ctrl_2_.reg_data
    };

    FM_PCF8553_LEGACY_Reset();
    fm_pcf8553_legacy_write_sequence_(FM_PCF8553_LEGACY_DEVICE_CTRL_ADDRESS,
                                      init_registers,
                                      (uint16_t) sizeof(init_registers));
}

void FM_PCF8553_LEGACY_Refresh(void)
{
    fm_pcf8553_legacy_write_sequence_(FM_PCF8553_LEGACY_DATA_ADDRESS,
                                      g_fm_pcf8553_legacy_ram_map,
                                      (uint16_t) sizeof(g_fm_pcf8553_legacy_ram_map));
}

void FM_PCF8553_LEGACY_Reset(void)
{
    FM_PORT_Pcf8553Ctrl_Reset();
    HAL_Delay(FM_PCF8553_LEGACY_POST_RESET_DELAY_MS);
}

void FM_PCF8553_LEGACY_WriteAll(uint8_t p_data)
{
    (void) memset(g_fm_pcf8553_legacy_ram_map, p_data, sizeof(g_fm_pcf8553_legacy_ram_map));
    FM_PCF8553_LEGACY_Refresh();
}

void FM_PCF8553_LEGACY_WriteByte(uint8_t p_address, uint8_t p_data)
{
    fm_pcf8553_legacy_write_sequence_(p_address, &p_data, 1U);
}

/*** end of file ***/

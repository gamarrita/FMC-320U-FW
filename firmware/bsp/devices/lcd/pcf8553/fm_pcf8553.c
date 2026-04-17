/**
 * @file    fm_pcf8553.c
 * @brief   Transitional PCF8553 LCD driver implementation.
 *
 * @details
 *  - This module is part of the commercial FMC-320U firmware product line and
 *    runs on the same board hardware used by the previous firmware.
 *  - The target MCU in this repository is STM32U575VIT6.
 *  - The main purpose of this repository is to rebuild the firmware almost
 *    from scratch using a new architecture and a VS Code + CMake workflow.
 *  - The product behavior exposed to the user is expected to remain aligned
 *    with the previous firmware, even though the internal code structure is
 *    being replaced.
 *  - The hardware did not change. What changed is the project organization,
 *    development workflow, and the repository layer model.
 *
 * Migration context:
 *  - This file was inherited from the previous firmware codebase.
 *  - Some inherited comments may describe the historical implementation rather
 *    than the desired architecture of this repository.
 *  - The current repository expects device behavior to live in `bsp/`, while
 *    MCU transport details should migrate toward `port/` backends.
 *  - Near-term objective: achieve a minimal PCF8553 bring-up on the current
 *    board, then progressively clean up the driver separation.
 *  - Expected refactor direction:
 *      1. isolate MCU bus access behind `port/` helpers
 *      2. keep device register logic in this module
 *      3. remove stale assumptions inherited from the former project layout
 */

#include "fm_pcf8553.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "fm_port_pcf8553_ctrl.h"
#include "fm_port_spi1.h"

/* Private Types */
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

/* Private Defines */
#define FM_PCF8553_SPI_TIMEOUT_MS             5U
#define FM_PCF8553_POST_RESET_DELAY_MS        5U
#define FM_PCF8553_DATA_ADDRESS               0x04U
#define FM_PCF8553_DEVICE_CTRL_ADDRESS        0x01U
#define FM_PCF8553_DISPLAY_CTRL_1_ADDRESS     0x02U
#define FM_PCF8553_DISPLAY_CTRL_2_ADDRESS     0x03U
#define FM_PCF8553_WRITE_DATA                 0U

/* Private Data */
uint8_t pcf8553_ram_map[PCF8553_RAM_SIZE];

static fm_pcf8553_device_ctrl_t g_device_ctrl =
{
    .reg_bits.clock_output = 0U,
    .reg_bits.internal_oscillator = 0U,
    .reg_bits.frame_frequency = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_display_ctrl_1_t g_display_ctrl_1 =
{
    .reg_bits.display_enabled = 1U,
    .reg_bits.bias_mode = 0U,
    .reg_bits.mux = 0U,
    .reg_bits.boost = 0U,
    .reg_bits.reserved = 0U
};

static fm_pcf8553_display_ctrl_2_t g_display_ctrl_2 =
{
    .reg_bits.inversion = 0U,
    .reg_bits.blink = 0U,
    .reg_bits.reserved = 0U
};

/* Private Prototypes */
static uint8_t fm_pcf8553_build_write_address_(uint8_t address);
static bool fm_pcf8553_write_transaction_(const uint8_t *p_data, uint16_t len);
static void fm_pcf8553_write_sequence_(uint8_t start_address, const uint8_t *p_data, uint16_t len);

/* Private Bodies */
static uint8_t fm_pcf8553_build_write_address_(uint8_t address)
{
    fm_pcf8553_register_address_t register_address = { 0 };

    register_address.bits.address = address;
    register_address.bits.not_used = 0U;
    register_address.bits.read_write = FM_PCF8553_WRITE_DATA;

    return register_address.data;
}

static bool fm_pcf8553_write_transaction_(const uint8_t *p_data, uint16_t len)
{
    bool success;

    if ((p_data == NULL) || (len == 0U))
    {
        return false;
    }

    FM_PORT_Pcf8553Ctrl_Enable();
    success = FM_PORT_Spi1_Write(p_data, len, FM_PCF8553_SPI_TIMEOUT_MS);
    FM_PORT_Pcf8553Ctrl_Disable();

    return success;
}

static void fm_pcf8553_write_sequence_(uint8_t start_address, const uint8_t *p_data, uint16_t len)
{
    uint8_t frame[PCF8553_RAM_SIZE + 1U];

    if ((p_data == NULL) || (len == 0U) || (len > PCF8553_RAM_SIZE))
    {
        Error_Handler();
        return;
    }

    frame[0] = fm_pcf8553_build_write_address_(start_address);
    (void) memcpy(&frame[1], p_data, len);

    if (!fm_pcf8553_write_transaction_(frame, (uint16_t)(len + 1U)))
    {
        Error_Handler();
    }
}

/* Public Bodies */
void FM_PCF8553_Blink(blink_t mode)
{
    g_display_ctrl_2.reg_bits.blink = (uint8_t) mode;
    fm_pcf8553_write_sequence_(FM_PCF8553_DISPLAY_CTRL_2_ADDRESS,
                               &g_display_ctrl_2.reg_data,
                               1U);
}

void FM_PCF8553_ClearBuffer(void)
{
    (void) memset(pcf8553_ram_map, PCF8553_SEGMENTS_OFF, sizeof(pcf8553_ram_map));
}

void FM_PCF8553_Init(void)
{
    uint8_t init_registers[] =
    {
        g_device_ctrl.reg_data,
        g_display_ctrl_1.reg_data,
        g_display_ctrl_2.reg_data
    };

    FM_PCF8553_Reset();
    fm_pcf8553_write_sequence_(FM_PCF8553_DEVICE_CTRL_ADDRESS,
                               init_registers,
                               (uint16_t) sizeof(init_registers));
}

void FM_PCF8553_Refresh(void)
{
    fm_pcf8553_write_sequence_(FM_PCF8553_DATA_ADDRESS,
                               pcf8553_ram_map,
                               (uint16_t) sizeof(pcf8553_ram_map));
}

void FM_PCF8553_Reset(void)
{
    FM_PORT_Pcf8553Ctrl_Reset();
    HAL_Delay(FM_PCF8553_POST_RESET_DELAY_MS);
}

void FM_PCF8553_WriteAll(uint8_t data)
{
    (void) memset(pcf8553_ram_map, data, sizeof(pcf8553_ram_map));
    FM_PCF8553_Refresh();
}

void FM_PCF8553_WriteByte(uint8_t add, uint8_t data)
{
    fm_pcf8553_write_sequence_(add, &data, 1U);
}

/*** end of file ***/

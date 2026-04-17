#include "fm_pcf8553_bringup.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_pcf8553_ctrl.h"
#include "fm_port_spi1.h"
#include "devices/lcd/pcf8553/fm_pcf8553.h"

#define PCF8553_SPI_TIMEOUT_MS           5U
#define PCF8553_RESET_DELAY_MS           5U
#define PCF8553_POST_INIT_DELAY_MS       10U
#define PCF8553_REG_DEVICE_CTRL          0x01U
#define PCF8553_REG_DISPLAY_DATA_START   0x04U

static void fm_pcf8553_bringup_log_(const char *p_msg)
{
    if (p_msg == NULL)
    {
        return;
    }

    (void) FM_DEBUG_UartMsg(p_msg, (uint32_t) strlen(p_msg));
}

static bool fm_pcf8553_bringup_write_transaction_(const uint8_t *p_data, uint16_t len)
{
    bool success;

    FM_PORT_Pcf8553Ctrl_Enable();
    success = FM_PORT_Spi1_Write(p_data, len, PCF8553_SPI_TIMEOUT_MS);
    FM_PORT_Pcf8553Ctrl_Disable();

    return success;
}

static bool fm_pcf8553_bringup_try_all_on_(void)
{
    uint8_t init_frame[] =
    {
        PCF8553_REG_DEVICE_CTRL,
        0x00U, /* Device_ctrl: default oscillator / frame settings for minimal hack. */
        0x01U, /* Display_ctrl_1: display enable. */
        0x00U  /* Display_ctrl_2: no inversion, no blink. */
    };
    uint8_t display_frame[PCF8553_RAM_SIZE + 1U];

    display_frame[0] = PCF8553_REG_DISPLAY_DATA_START;
    (void) memset(&display_frame[1], PCF8553_SEGMENTS_ON, PCF8553_RAM_SIZE);

    FM_PORT_Pcf8553Ctrl_Reset();
    HAL_Delay(PCF8553_RESET_DELAY_MS);

    if (!fm_pcf8553_bringup_write_transaction_(init_frame, (uint16_t) sizeof(init_frame)))
    {
        return false;
    }

    HAL_Delay(PCF8553_POST_INIT_DELAY_MS);

    return fm_pcf8553_bringup_write_transaction_(display_frame, (uint16_t) sizeof(display_frame));
}

void FM_Pcf8553Bringup_Run(void)
{
    bool activated;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    fm_pcf8553_bringup_log_("PCF8553 bring-up: SPI1 init sequence\n");
    activated = fm_pcf8553_bringup_try_all_on_();

    if (!activated)
    {
        fm_pcf8553_bringup_log_("PCF8553 bring-up: SPI1 write sequence failed\n");
        FM_DEBUG_LedError(FM_DEBUG_LED_ON);
    }
    else
    {
        fm_pcf8553_bringup_log_("PCF8553 bring-up: all-segments-on sent over SPI1\n");
        FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    }

    for (;;)
    {
        FM_DEBUG_Flush();
        HAL_Delay(100U);
    }
}

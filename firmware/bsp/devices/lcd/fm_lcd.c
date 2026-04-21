/**
 * @file    fm_lcd.c
 * @brief   Public V1 LCD implementation for the redesign path.
 */

#include "fm_lcd.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "fm_lcd_map.h"
#include "pcf8553/fm_pcf8553.h"

#if (FM_LCD_MAP_RAM_SIZE != FM_PCF8553_RAM_SIZE)
#error "LCD map and PCF8553 RAM sizes must match."
#endif

/* =========================== Private Types ============================== */
typedef struct
{
    bool initialized;
    bool dirty;
    bool hw_synced;
    bool blink_enabled;
    fm_lcd_blink_phase_t blink_phase;
    uint8_t blink_range_count;
    fm_lcd_blink_range_t blink_ranges[FM_LCD_BLINK_RANGE_MAX];
    uint8_t desired_ram[FM_LCD_MAP_RAM_SIZE];
    uint8_t flushed_ram[FM_LCD_MAP_RAM_SIZE];
} fm_lcd_context_t;

/* =========================== Private Data =============================== */
static fm_lcd_context_t g_fm_lcd_context;

/* =========================== Private Prototypes ========================= */
static uint8_t fm_lcd_get_row_columns_(fm_lcd_layout_row_t p_row);
static uint8_t fm_lcd_get_text_field_columns_(fm_lcd_text_field_t p_field);
static fm_lcd_status_t fm_lcd_from_map_status_(fm_lcd_map_status_t p_status);
static fm_lcd_status_t fm_lcd_from_backend_status_(fm_pcf8553_status_t p_status);
static fm_lcd_status_t fm_lcd_validate_blink_range_(const fm_lcd_blink_range_t *p_range);
static void fm_lcd_refresh_dirty_flag_(void);
static bool fm_lcd_find_dirty_range_(uint8_t *p_first, uint8_t *p_len);

/* =========================== Private Bodies ============================= */
static uint8_t fm_lcd_get_row_columns_(fm_lcd_layout_row_t p_row)
{
    switch (p_row)
    {
    case FM_LCD_LAYOUT_ROW_TOP:
        return FM_LCD_LAYOUT_TOP_ROW_COLUMNS;
    case FM_LCD_LAYOUT_ROW_BOTTOM:
        return FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS;
    default:
        return 0U;
    }
}

static uint8_t fm_lcd_get_text_field_columns_(fm_lcd_text_field_t p_field)
{
    switch (p_field)
    {
    case FM_LCD_TEXT_FIELD_TOP_ROW:
        return FM_LCD_LAYOUT_TOP_ROW_COLUMNS;
    case FM_LCD_TEXT_FIELD_BOTTOM_ROW:
        return FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS;
    case FM_LCD_TEXT_FIELD_ALPHA:
        return FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT;
    default:
        return 0U;
    }
}

static fm_lcd_status_t fm_lcd_from_map_status_(fm_lcd_map_status_t p_status)
{
    switch (p_status)
    {
    case FM_LCD_MAP_OK:
        return FM_LCD_OK;
    case FM_LCD_MAP_EINVAL:
        return FM_LCD_EINVAL;
    case FM_LCD_MAP_ERANGE:
    case FM_LCD_MAP_ENOTSUP:
        return FM_LCD_ERANGE;
    default:
        return FM_LCD_ESTATE;
    }
}

static fm_lcd_status_t fm_lcd_from_backend_status_(fm_pcf8553_status_t p_status)
{
    switch (p_status)
    {
    case FM_PCF8553_OK:
        return FM_LCD_OK;
    case FM_PCF8553_EINVAL:
        return FM_LCD_EINVAL;
    case FM_PCF8553_ERANGE:
        return FM_LCD_ERANGE;
    case FM_PCF8553_ESTATE:
        return FM_LCD_ESTATE;
    case FM_PCF8553_EIO:
    default:
        return FM_LCD_EIO;
    }
}

static fm_lcd_status_t fm_lcd_validate_blink_range_(const fm_lcd_blink_range_t *p_range)
{
    uint8_t field_columns;

    if (p_range == NULL)
    {
        return FM_LCD_EINVAL;
    }

    if (p_range->length == 0U)
    {
        return FM_LCD_EINVAL;
    }

    field_columns = fm_lcd_get_text_field_columns_(p_range->field);

    if (field_columns == 0U)
    {
        return FM_LCD_ERANGE;
    }

    if (p_range->start >= field_columns)
    {
        return FM_LCD_ERANGE;
    }

    if ((uint16_t) p_range->start + (uint16_t) p_range->length > (uint16_t) field_columns)
    {
        return FM_LCD_ERANGE;
    }

    return FM_LCD_OK;
}

static void fm_lcd_refresh_dirty_flag_(void)
{
    g_fm_lcd_context.dirty =
        (memcmp(g_fm_lcd_context.desired_ram,
                g_fm_lcd_context.flushed_ram,
                FM_LCD_MAP_RAM_SIZE) != 0);
}

static bool fm_lcd_find_dirty_range_(uint8_t *p_first, uint8_t *p_len)
{
    uint8_t index;
    uint8_t first;
    uint8_t last;
    bool found;

    if ((p_first == NULL) || (p_len == NULL))
    {
        return false;
    }

    found = false;
    first = 0U;
    last = 0U;

    for (index = 0U; index < FM_LCD_MAP_RAM_SIZE; index++)
    {
        if (g_fm_lcd_context.desired_ram[index] != g_fm_lcd_context.flushed_ram[index])
        {
            if (!found)
            {
                first = index;
                last = index;
                found = true;
            }
            else
            {
                last = index;
            }
        }
    }

    if (!found)
    {
        return false;
    }

    *p_first = first;
    *p_len = (uint8_t)((last - first) + 1U);

    return true;
}

/* =========================== Public Bodies ============================== */
fm_lcd_status_t FM_LCD_Init(void)
{
    fm_lcd_map_status_t map_status;
    fm_pcf8553_status_t backend_status;

    (void) memset(&g_fm_lcd_context, 0, sizeof(g_fm_lcd_context));

    backend_status = FM_PCF8553_Init();

    if (backend_status != FM_PCF8553_OK)
    {
        return fm_lcd_from_backend_status_(backend_status);
    }

    map_status = FM_LCD_MAP_Clear(g_fm_lcd_context.desired_ram,
                                  (uint8_t)sizeof(g_fm_lcd_context.desired_ram));

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    (void) memset(g_fm_lcd_context.flushed_ram, 0, sizeof(g_fm_lcd_context.flushed_ram));
    g_fm_lcd_context.initialized = true;
    g_fm_lcd_context.dirty = false;
    g_fm_lcd_context.hw_synced = false;

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_Clear(void)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_Clear(g_fm_lcd_context.desired_ram,
                                  (uint8_t)sizeof(g_fm_lcd_context.desired_ram));

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_ClearRow(fm_lcd_layout_row_t p_row)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_ClearRow(g_fm_lcd_context.desired_ram,
                                     (uint8_t)sizeof(g_fm_lcd_context.desired_ram),
                                     p_row);

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_ClearAlpha(void)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_ClearAlpha(g_fm_lcd_context.desired_ram,
                                       (uint8_t) sizeof(g_fm_lcd_context.desired_ram));

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_BlinkClear(void)
{
    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    g_fm_lcd_context.blink_enabled = false;
    g_fm_lcd_context.blink_phase = FM_LCD_BLINK_PHASE_ON;
    g_fm_lcd_context.blink_range_count = 0U;
    (void) memset(g_fm_lcd_context.blink_ranges, 0, sizeof(g_fm_lcd_context.blink_ranges));

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_BlinkSetRanges(const fm_lcd_blink_range_t *p_ranges,
                                      uint8_t p_range_count)
{
    fm_lcd_status_t status;
    uint8_t index;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    if ((p_ranges == NULL) || (p_range_count == 0U) ||
        (p_range_count > FM_LCD_BLINK_RANGE_MAX))
    {
        return FM_LCD_EINVAL;
    }

    for (index = 0U; index < p_range_count; index++)
    {
        status = fm_lcd_validate_blink_range_(&p_ranges[index]);

        if (status != FM_LCD_OK)
        {
            return status;
        }
    }

    (void) memcpy(g_fm_lcd_context.blink_ranges,
                  p_ranges,
                  (size_t) p_range_count * sizeof(g_fm_lcd_context.blink_ranges[0]));

    if (p_range_count < FM_LCD_BLINK_RANGE_MAX)
    {
        (void) memset(&g_fm_lcd_context.blink_ranges[p_range_count],
                      0,
                      (size_t) (FM_LCD_BLINK_RANGE_MAX - p_range_count) *
                          sizeof(g_fm_lcd_context.blink_ranges[0]));
    }

    g_fm_lcd_context.blink_range_count = p_range_count;

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_BlinkSetEnabled(bool p_enabled)
{
    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    if (p_enabled && (g_fm_lcd_context.blink_range_count == 0U))
    {
        return FM_LCD_ESTATE;
    }

    g_fm_lcd_context.blink_enabled = p_enabled;

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_BlinkSetPhase(fm_lcd_blink_phase_t p_phase)
{
    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    if ((p_phase != FM_LCD_BLINK_PHASE_ON) &&
        (p_phase != FM_LCD_BLINK_PHASE_OFF))
    {
        return FM_LCD_EINVAL;
    }

    g_fm_lcd_context.blink_phase = p_phase;

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_WriteText(fm_lcd_layout_row_t p_row,
                                 const char *p_text,
                                 fm_lcd_align_t p_align,
                                 bool p_clear_rest)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_WriteText(g_fm_lcd_context.desired_ram,
                                      (uint8_t)sizeof(g_fm_lcd_context.desired_ram),
                                      p_row,
                                      p_text,
                                      p_align,
                                      p_clear_rest);

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_WriteAlpha(const char *p_text,
                                  fm_lcd_align_t p_align,
                                  bool p_clear_rest)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_WriteAlpha(g_fm_lcd_context.desired_ram,
                                       (uint8_t) sizeof(g_fm_lcd_context.desired_ram),
                                       p_text,
                                       p_align,
                                       p_clear_rest);

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_SetIndicator(fm_lcd_layout_indicator_t p_indicator,
                                    bool p_on)
{
    fm_lcd_map_status_t map_status;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    map_status = FM_LCD_MAP_SetIndicator(g_fm_lcd_context.desired_ram,
                                         (uint8_t)sizeof(g_fm_lcd_context.desired_ram),
                                         p_indicator,
                                         p_on);

    if (map_status != FM_LCD_MAP_OK)
    {
        return fm_lcd_from_map_status_(map_status);
    }

    fm_lcd_refresh_dirty_flag_();

    return FM_LCD_OK;
}

fm_lcd_status_t FM_LCD_Flush(void)
{
    fm_pcf8553_status_t backend_status;
    uint8_t first_changed;
    uint8_t changed_len;

    if (!g_fm_lcd_context.initialized)
    {
        return FM_LCD_ESTATE;
    }

    if ((!g_fm_lcd_context.dirty) && g_fm_lcd_context.hw_synced)
    {
        return FM_LCD_OK;
    }

    if (!g_fm_lcd_context.hw_synced)
    {
        first_changed = 0U;
        changed_len = FM_LCD_MAP_RAM_SIZE;
    }
    else
    {
        if (!fm_lcd_find_dirty_range_(&first_changed, &changed_len))
        {
            g_fm_lcd_context.dirty = false;
            return FM_LCD_OK;
        }
    }

    backend_status = FM_PCF8553_WriteRam(first_changed,
                                         &g_fm_lcd_context.desired_ram[first_changed],
                                         changed_len);

    if (backend_status != FM_PCF8553_OK)
    {
        return fm_lcd_from_backend_status_(backend_status);
    }

    (void) memcpy(g_fm_lcd_context.flushed_ram,
                  g_fm_lcd_context.desired_ram,
                  sizeof(g_fm_lcd_context.flushed_ram));
    g_fm_lcd_context.dirty = false;
    g_fm_lcd_context.hw_synced = true;

    return FM_LCD_OK;
}

bool FM_LCD_IsDirty(void)
{
    if (!g_fm_lcd_context.initialized)
    {
        return false;
    }

    return g_fm_lcd_context.dirty;
}

uint8_t FM_LCD_GetRowColumns(fm_lcd_layout_row_t p_row)
{
    return fm_lcd_get_row_columns_(p_row);
}

/*** end of file ***/

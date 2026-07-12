/**
 * @file fm_debug.c
 * @brief Debug services: ITM tracing, UART logging, and diagnostic LEDs.
 *
 * The module enables hardware-controlled debug features when the corresponding
 * jumpers are fitted and offers convenience helpers to publish messages over
 * ITM or UART1. It also manages three status LEDs (error, activity, signal)
 * frequently used during field validation.
 */

#include "fm_debug.h"
#include "usart.h"

// --- Internal constants ---

#define WAIT_FOR_UART_5MS   (5)
#define WAIT_FOR_UART_1MS   (1)

#define TRUE    (1)
#define FALSE   (0)

// Maximum string lengths when formatting numeric values.
#define SIZE_OF_UINT8   (4)   // "255"
#define SIZE_OF_INT8    (5)   // "-128"
#define SIZE_OF_UINT16  (6)   // "65534"
#define SIZE_OF_INT16   (7)   // "-32768"
#define SIZE_OF_UINT32  (11)  // "4294967295"
#define SIZE_OF_INT32   (12)  // "-2147483648"
#define SIZE_OF_FLOAT   (10)  // "-123456.7"

#define FM_DEBUG_UART_MSG      // General-purpose messages
#define FM_DEBUG_UART_ERROR    // Error messages

// --- External dependencies ---

extern UART_HandleTypeDef huart1;

// --- Internal state ---

static int  debug_uart_enable;
static int  debug_led_enable;
static char str_buffer[SIZE_OF_INT32];
static const char FM_DEBUG_ASCII_CR[] = "\n";

// --- API ---

/**
 * Reads the configuration jumpers and enables the corresponding debug features.
 *
 * PD1 controls the diagnostic LEDs, while PD2 enables the UART1 debug console.
 * If a jumper is open the related feature is disabled and the hardware placed in
 * a low-power configuration.
 */
void FM_DEBUG_Init(void)
{
    char debug_on_msg[] = "Debug ON";
    GPIO_InitTypeDef gpio_init = {0};
    GPIO_PinState leds_enable_pin;
    GPIO_PinState uart_enable_pin;

    gpio_init.Pin = DEBUG_LED_Pin;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DEBUG_LED_GPIO_Port, &gpio_init);

    gpio_init.Pin = DEBUG_UART_Pin;
    HAL_GPIO_Init(DEBUG_UART_GPIO_Port, &gpio_init);

    leds_enable_pin = HAL_GPIO_ReadPin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
    uart_enable_pin = HAL_GPIO_ReadPin(DEBUG_UART_GPIO_Port, DEBUG_UART_Pin);

    if (leds_enable_pin == GPIO_PIN_RESET) {
        debug_led_enable = TRUE;
    } else {
        HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_RESET);
        debug_led_enable = FALSE;
    }

    if (uart_enable_pin == GPIO_PIN_RESET) {
        MX_USART1_UART_Init();
        debug_uart_enable = TRUE;
        FM_DEBUG_UartMsg(debug_on_msg, (uint8_t)sizeof(debug_on_msg));
    } else {
        HAL_UART_DeInit(&huart1);
        debug_uart_enable = FALSE;
    }

    gpio_init.Pin = DEBUG_LED_Pin;
    gpio_init.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(DEBUG_LED_GPIO_Port, &gpio_init);

    gpio_init.Pin = DEBUG_UART_Pin;
    HAL_GPIO_Init(DEBUG_UART_GPIO_Port, &gpio_init);
}

/**
 * Sends a message through the ITM port for display in the debugger console.
 * @param msg Pointer to the character buffer.
 * @param len Number of characters to send from the buffer.
 */
void FM_DEBUG_ItmMsg(const char *msg, uint8_t len)
{
    while (*msg && len) {
        ITM_SendChar((uint32_t)(*msg));
        --len;
        ++msg;
    }
}

/**
 * Controls the error LED according to the requested state.
 * @param led_status Non-zero turns the LED on when debug mode is active.
 */
void FM_DEBUG_LedError(int led_status)
{
    if (debug_led_enable && led_status) {
        HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
    }

#ifdef STOP_ON_ERROR
#warning "Warning: disable STOP_ON_ERROR for beta or production builds"
    while (STOP_ON_ERROR) {
    }
#endif
}

/**
 * Controls the activity LED according to the requested state.
 * @param led_status Non-zero turns the LED on when debug mode is active.
 */
void FM_DEBUG_LedActive(int led_status)
{
    if (debug_led_enable && led_status) {
        HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_RESET);
    }
}

/**
 * Controls the signal LED according to the requested state.
 * @param led_status Non-zero turns the LED on when debug mode is active.
 */
void FM_DEBUG_LedSignal(int led_status)
{
    if (debug_led_enable && led_status) {
        HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_RESET);
    }
}

/**
 * Sends a debug message over UART and appends a line feed.
 * @param p_msg Pointer to the buffer.
 * @param len   Number of valid bytes inside the buffer.
 */
void FM_DEBUG_UartMsg(const char *p_msg, uint8_t len)
{
    HAL_StatusTypeDef ret;

    if (!debug_uart_enable || len < 1u) {
        return;
    }

    ret = HAL_UART_Transmit(&huart1, (const uint8_t *)p_msg, len, WAIT_FOR_UART_5MS);
    if (ret != HAL_BUSY) {
        HAL_UART_Transmit(&huart1, (const uint8_t *)FM_DEBUG_ASCII_CR, 1u, WAIT_FOR_UART_5MS);
    }
}

/**
 * Sends an unsigned 8-bit integer through the debug UART.
 * @param num Value to print.
 */
void FM_DEBUG_UartUint8(uint8_t num)
{
    if (!debug_uart_enable) {
        return;
    }

    snprintf(str_buffer, sizeof(str_buffer), "%u\n", num);
    FM_DEBUG_UartMsg(str_buffer, (uint8_t)strlen(str_buffer));
}

/**
 * Sends an unsigned 16-bit integer through the debug UART.
 * @param num Value to print.
 */
void FM_DEBUG_UartUint16(uint16_t num)
{
    if (!debug_uart_enable) {
        return;
    }

    snprintf(str_buffer, sizeof(str_buffer), "%u\n", num);
    FM_DEBUG_UartMsg(str_buffer, (uint8_t)strlen(str_buffer));
}

/**
 * Sends an unsigned 32-bit integer through the debug UART.
 * @param num Value to print.
 */
void FM_DEBUG_UartUint32(uint32_t num)
{
    if (!debug_uart_enable) {
        return;
    }

    snprintf(str_buffer, sizeof(str_buffer), "%lu\n", (unsigned long)num);
    FM_DEBUG_UartMsg(str_buffer, (uint8_t)strlen(str_buffer));
}

/**
 * Sends a signed 32-bit integer through the debug UART.
 * @param num Value to print.
 */
void FM_DEBUG_UartInt32(int32_t num)
{
    if (!debug_uart_enable) {
        return;
    }

    snprintf(str_buffer, sizeof(str_buffer), "%ld\n", (long)num);
    FM_DEBUG_UartMsg(str_buffer, (uint8_t)strlen(str_buffer));
}

/**
 * Sends a floating-point number with two decimals through the debug UART.
 * @param num Value to print.
 */
void FM_DEBUG_UartFloat(float num)
{
    if (!debug_uart_enable) {
        return;
    }

    snprintf(str_buffer, SIZE_OF_FLOAT, "%0.2f\n", (double)num);
    FM_DEBUG_UartMsg(str_buffer, SIZE_OF_FLOAT);
}


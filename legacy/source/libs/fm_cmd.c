/**
 * @file fm_cmd.c
 * @brief FM+ command processor implementation.
 *
 * A dedicated ThreadX task receives complete FM+ lines from UART3, matches
 * them against the command table and executes the corresponding response.
 */

#include "fm_cmd.h"
#include <string.h>
#include <stdio.h>

// --- Internal constants ---

#define UART3_TX_BUFFER_SIZE   (32u)
#define NUM_COMMANDS           (sizeof(fm_commands) / sizeof(fm_commands[0]))

// --- Internal state ---

static int temperature = 25; ///< Simulated temperature reading.

static const fm_cmd_entry_t fm_commands[] = {
    { "FM+VERSION?",  FM_CMD_TYPE_LITERAL,  .response.literal = "FMC-320U" },
    { "FM+TEMP?",     FM_CMD_TYPE_HANDLER,  .response.handler = FM_CMD_HandleTemp },
    { "FM+LOG_ALL?",  FM_CMD_TYPE_DEFERRED, .response.handler = FM_CMD_HandleLogAll },
    { "FM+COUNT?",    FM_CMD_TYPE_HANDLER,  .response.handler = FM_CMD_HandleCount },
};

static TX_THREAD cmd_thread; ///< Thread in charge of processing FM+ commands.
static TX_QUEUE  cmd_queue;  ///< Queue that stores incoming FM+ lines.
static uint8_t   cmd_queue_buffer[3u * FM_CMD_BYTE_SIZE];

// --- Private prototypes ---

static void process_line_(const char *line);

// --- Private functions ---

/**
 * Matches an incoming command against the table and triggers the configured response.
 * @param line Raw line as received, including the FM+ verb and optional arguments.
 */
static void process_line_(const char *line)
{
    for (size_t i = 0; i < NUM_COMMANDS; ++i) {
        const fm_cmd_entry_t *entry = &fm_commands[i];
        size_t cmd_len = strlen(entry->command);
        if (strncmp(line, entry->command, cmd_len) != 0) {
            continue;
        }

        if (entry->type == FM_CMD_TYPE_LITERAL) {
            HAL_UART_Transmit_DMA(&huart3, (uint8_t *)entry->response.literal, strlen(entry->response.literal));
        } else {
            entry->response.handler(line);
        }
        break;
    }
}

// --- Public handlers ---

/**
 * Replies with a mock temperature reading.
 * @param args Optional argument string (unused).
 */
void FM_CMD_HandleTemp(const char *args)
{
    (void)args;
    static char response[UART3_TX_BUFFER_SIZE];
    snprintf(response, sizeof(response), "TEMP:%d\r\n", temperature);
    HAL_UART_Transmit_DMA(&huart3, (uint8_t *)response, strlen(response));
}

/**
 * Sends a short list of log identifiers via DMA.
 * @param args Optional argument string (unused).
 */
void FM_CMD_HandleLogAll(const char *args)
{
    (void)args;
    static const char *log_lines[] = { "LOG1\r\n", "LOG2\r\n", "LOG3\r\n" };

    for (size_t i = 0; i < (sizeof(log_lines) / sizeof(log_lines[0])); ++i) {
        HAL_UART_Transmit_DMA(&huart3, (uint8_t *)log_lines[i], strlen(log_lines[i]));
    }
}

/**
 * Reports a mock counter using interrupt-driven transmission.
 * @param args Optional argument string (unused).
 */
void FM_CMD_HandleCount(const char *args)
{
    (void)args;
    static const int count = 42;
    char buf[32];
    snprintf(buf, sizeof(buf), "COUNT:%d\r\n", count);
    HAL_UART_Transmit_IT(&huart3, (uint8_t *)buf, strlen(buf));
}

// --- API ---

/**
 * Creates the command thread and queue using the provided ThreadX byte pool.
 * @param memory_ptr Pointer to the byte pool used for dynamic allocations.
 */
void FM_CMD_RtosInit(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
    CHAR *stack_ptr = NULL;

    if (tx_byte_allocate(byte_pool, (VOID **)&stack_ptr, FMX_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1) { }
    }

    if (tx_thread_create(&cmd_thread,
                         "CMD_THREAD",
                         FM_CMD_ThreadEntry,
                         (ULONG)&cmd_queue,
                         stack_ptr,
                         FMX_STACK_SIZE,
                         FMX_THREAD_PRIORITY_10,
                         FMX_THRESHOLD_10,
                         FMX_SLICE_0,
                         TX_AUTO_START) != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1) { }
    }

    if (tx_queue_create(&cmd_queue, "CMD_QUEUE", FM_CMD_ULONG_SIZE, cmd_queue_buffer, sizeof(cmd_queue_buffer)) != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1) { }
    }
}

/**
 * Entry point for the ThreadX task that consumes the command queue.
 * @param input Queue address passed at thread creation.
 */
void FM_CMD_ThreadEntry(ULONG input)
{
    TX_QUEUE *queue = (TX_QUEUE *)input;
    fm_cmd_command_t cmd;

    for (;;) {
        if (tx_queue_receive(queue, &cmd, TX_WAIT_FOREVER) == TX_SUCCESS) {
            process_line_(cmd.line);
        }
    }
}


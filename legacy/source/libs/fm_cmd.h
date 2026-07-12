/**
 * @file fm_cmd.h
 * @brief FM+ command processor interface over UART.
 *
 * The module exposes helpers to bootstrap the ThreadX command thread and the
 * handlers that implement every supported FM+ command.
 */

#ifndef FM_CMD_H_
#define FM_CMD_H_

#include "tx_api.h"
#include "main.h"
#include "usart.h"
#include "fmx.h"
#include "fm_debug.h"

// --- Constants ---

/** Maximum command payload expressed in ULONG words (ThreadX queue granularity). */
#define FM_CMD_ULONG_SIZE   (16u)

/** Command payload in bytes (ULONG * FM_CMD_ULONG_SIZE). */
#define FM_CMD_BYTE_SIZE    (sizeof(ULONG) * FM_CMD_ULONG_SIZE)

// --- Types ---

/** Scratch buffer that stores one incoming FM+ line. */
typedef struct {
    char line[FM_CMD_BYTE_SIZE];
} fm_cmd_command_t;

/** Response strategy for a command entry. */
typedef enum {
    FM_CMD_TYPE_LITERAL,   ///< The reply is a fixed literal string.
    FM_CMD_TYPE_HANDLER,   ///< A handler runs immediately in the command thread.
    FM_CMD_TYPE_DEFERRED   ///< A handler may defer the reply (DMA, other task, etc.).
} fm_cmd_type_t;

/** Mapping between a textual command and the action to perform. */
typedef struct {
    const char      *command;
    fm_cmd_type_t    type;
    union {
        const char *literal;              ///< Literal response terminated with CRLF.
        void (*handler)(const char *args);///< Callback executed when the command arrives.
    } response;
} fm_cmd_entry_t;

// --- API ---

void FM_CMD_RtosInit(VOID *memory_ptr);
void FM_CMD_ThreadEntry(ULONG input);
void FM_CMD_HandleTemp(const char *args);
void FM_CMD_HandleLogAll(const char *args);
void FM_CMD_HandleCount(const char *args);

#endif // FM_CMD_H_


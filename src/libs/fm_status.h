/**
 * @file    fm_status.h
 * @brief   Common lightweight status codes for authored firmware modules.
 */
#ifndef FM_STATUS_H
#define FM_STATUS_H

typedef enum
{
    /** Operation completed successfully. */
    FM_STATUS_OK = 0,
    /** Invalid argument, unsupported enum value, or invalid pointer. */
    FM_STATUS_EINVAL,
    /** Value is outside the supported numeric range. */
    FM_STATUS_ERANGE,
    /** Module state does not allow the requested operation. */
    FM_STATUS_ESTATE,
    /** Valid request is not supported by the current implementation slice. */
    FM_STATUS_ENOTSUP
} fm_status_t;

#endif /* FM_STATUS_H */

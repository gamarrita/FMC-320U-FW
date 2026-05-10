/**
 * @file    fm_status.h
 * @brief   Common lightweight status codes for authored firmware modules.
 */
#ifndef FM_STATUS_H
#define FM_STATUS_H

typedef enum
{
    FM_STATUS_OK = 0,
    FM_STATUS_EINVAL,
    FM_STATUS_ERANGE,
    FM_STATUS_ESTATE,
    FM_STATUS_ENOTSUP
} fm_status_t;

#endif /* FM_STATUS_H */

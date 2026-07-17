#ifndef APP_ENTRY_H
#define APP_ENTRY_H

/**
 * @file    app_entry.h
 * @brief   App entry point for the display-format/LCD bring-up profile.
 */

/**
 * @brief Run the display-format/LCD bring-up selected by this app profile.
 *
 * Delegates to `FM_DisplayFormatLcdBringup_Run()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void APP_ENTRY_Run(void);

#endif /* APP_ENTRY_H */

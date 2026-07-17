#ifndef APP_ENTRY_H_
#define APP_ENTRY_H_

/**
 * @file    app_entry.h
 * @brief   App entry point for the LCD bring-up profile.
 */

/**
 * @brief Run the LCD bring-up selected by this app profile.
 *
 * Delegates to `FM_LcdBringup_Run()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void APP_ENTRY_Run(void);

#endif /* APP_ENTRY_H_ */

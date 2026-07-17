/**
 * @file    app_entry.h
 * @brief   App entry point for the template profile.
 */
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

/**
 * @brief Run the template app selected by this app profile.
 *
 * Delegates to `FM_Template_Run()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void APP_ENTRY_Run(void);

#endif /* APP_ENTRY_H */

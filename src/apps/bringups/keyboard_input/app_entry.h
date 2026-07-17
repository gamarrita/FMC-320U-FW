/**
 * @file    app_entry.h
 * @brief   Active-app entry contract used by the generated main file.
 *
 * This profile adapts generated startup glue to the keyboard input bring-up.
 */
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

/**
 * @brief Run the selected firmware app.
 *
 * Delegates to `FM_KeyboardInputBringup_Run()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void APP_ENTRY_Run(void);

#endif /* APP_ENTRY_H */

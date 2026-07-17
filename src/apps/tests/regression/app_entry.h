/**
 * @file    app_entry.h
 * @brief   App entry point for the shared regression test profile.
 *
 * This header is included by generated startup glue to run the selected app
 * without exposing that glue to app internals.
 */
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

/**
 * @brief Run the regression-test app selected by this app profile.
 *
 * Delegates to `FM_RegressionTest_Run()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void APP_ENTRY_Run(void);

#endif /* APP_ENTRY_H */

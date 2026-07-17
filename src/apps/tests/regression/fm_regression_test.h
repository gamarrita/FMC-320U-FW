/**
 * @file    fm_regression_test.h
 * @brief   Shared authored-firmware regression test app.
 *
 * This app runs the repository's firmware regression checks on target hardware
 * and reports progress over the debug UART. It initializes the shared board and
 * debug services itself.
 */
#ifndef FM_REGRESSION_TEST_H
#define FM_REGRESSION_TEST_H

/**
 * @brief Run all regression cases and then remain in an idle indication loop.
 *
 * On the first failing case, this function emits failure context, flushes
 * debug output, and enters the shared panic path. On success, it emits a pass
 * marker and blinks the run LED forever.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_RegressionTest_Run(void);

#endif /* FM_REGRESSION_TEST_H */

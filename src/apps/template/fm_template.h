/**
 * @file    fm_template.h
 * @brief   Minimal selectable-app template contract.
 *
 * The template app owns no hardware behavior. It exists as a starting point for
 * new app profiles.
 */
#ifndef FM_TEMPLATE_H_
#define FM_TEMPLATE_H_

/**
 * @brief Run the template app.
 *
 * The current template enters an empty foreground loop forever.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_Template_Run(void);

#endif /* FM_TEMPLATE_H_ */

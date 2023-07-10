#ifndef FAN_H
#define FAN_H

#include <stdint.h>

/* Pin mappings
Fan -- STM8S   
YELLOW -> PD4 (Timer 2 channel oc 1)
*/

/* Configure TIM2 for PWM mode on TIM2 channel 1 */
void PWM_init();

/* Adjustable from 1-80 as 0%-100% PWM cycle
   Actual fan rpm will be different at low %'s since fans have minimum rpms */
void PWM_set_duty_cycle(uint8_t duty_cycle);

/* Starts the TIM2 timer which generates the PWM signal */
void PWM_start();

/* Stop PWM output */
void PWM_stop();

#endif
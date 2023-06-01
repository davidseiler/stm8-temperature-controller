#include "fan.h"
#include "stm8s.h"

#define PWM_PERIOD   0x0050

void PWM_init() {
    // PWM frequency = (MASTER / 2^(Prescale value)) / Auto Reload Register
    // To control the 4-wire fan a frequency of 25 kHz is required (within 21 kHz and 28 kHz is allowed)
    TIM2_PSCR = 0b0000;
    TIM2_ARRH = (PWM_PERIOD >> 8);
    TIM2_ARRL = PWM_PERIOD;

    // Enable pwm mode, OC1PE set to 1 since preload register is set in TIM2_CR1, set channel as output
    TIM2_CCMR1 |= 0b01101000;     
    // Enable the preload registers for output channel 
    TIM2_EGR |= 0x01;    
    // Set channel as output to pin TIM2_OC1 channel which is PD4        
    TIM2_CCER1 |= 0b00000001;     
}

void PWM_stop() {
    TIM2_CR1 &= ~(0b10000001);
}

void PWM_start() {
    TIM2_CR1 |= 0b10000001; // Enable ARPE and start timer
}

// With the default master clock of 2MHz the possible values are 1-80
void PWM_set_duty_cycle(uint8_t duty_cycle) {
    TIM2_CCR1H = ((duty_cycle) >> 8);
    TIM2_CCR1L = (duty_cycle);
}
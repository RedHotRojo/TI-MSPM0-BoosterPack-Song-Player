#ifndef TIMER_H_
#define TIMER_H_

#include <ti/devices/msp/msp.h>
#include "bsp.h"

/*!
 *  @brief  Configuration struct for timer clock configuration
 */
typedef struct {
    uint32_t clockSel;      // Selects timer module clock source
    uint32_t divideRatio;   // Clock divider is divideRatio + 1.
    uint8_t prescale;       // Clock PreScale (CPS) division is prescale + 1
} Timer_ClockConfig;

/*!
 * @brief Configuration struct for timer configuration.
 */
typedef struct {
    uint32_t timerMode; // One shot or Periodic mode configuration.
    uint32_t period;    // Timer load value
} Timer_TimerConfig;

typedef struct {
    uint32_t imask;
    uint32_t priority;
    IRQn_Type IRQnum;
} Timer_InterruptConfig;

typedef struct {
    uint8_t index;      // Index for pin control management register
    uint32_t iomuxMode; // IOMUX mode select for pin
    uint8_t ccr;        // Compare/Capture register
    uint32_t ccpd;      // CCP direction (input or output)
    uint32_t ccctl;     // Compare/Capture control register value
    uint32_t octl;      // Output control
    uint32_t ccact;     // Compare/Capture action
    uint32_t duty;      // Compare/Capture register value establishes duty cycle
} Timer_PWMConfig;

void InitializeTimerClock(GPTIMER_Regs *timer, Timer_ClockConfig *clock_config);
void InitializeTimerPWM(GPTIMER_Regs *timer, Timer_PWMConfig *pwm_config);
void InitializeTimerCompare(GPTIMER_Regs *timer, Timer_TimerConfig *pwm_config);
void TimeDelay(GPTIMER_Regs *timer, Timer_TimerConfig *timer_config);
void UpdateDutyCycle(GPTIMER_Regs *timer,Timer_PWMConfig *pwm_config);
void EnableTimer(GPTIMER_Regs *timer);
void InitializeTimerInterrupt(GPTIMER_Regs *timer, Timer_InterruptConfig *interrupt_config);

#endif /* TIMER_H_ */

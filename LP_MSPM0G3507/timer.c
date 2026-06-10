#include "timer.h"

void InitializeTimerClock(GPTIMER_Regs *timer, Timer_ClockConfig *clock_config)
{
    // The timer must first be reset and power enabled before configuration.
    timer->GPRCM.RSTCTL = (GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT | GPTIMER_RSTCTL_RESETSTKYCLR_CLR);
    timer->GPRCM.PWREN = (GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE);

    // Delay for power startup.
    ClockDelay(POWER_STARTUP_DELAY);

    // Assign clock select, clock divide, and pre-scale values
    timer->CLKSEL = clock_config->clockSel;
    timer->CLKDIV = clock_config->divideRatio;
    timer->COMMONREGS.CPS = clock_config->prescale;

    // Finally, enable the timer clock.
    timer->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
}

void InitializeTimerPWM(GPTIMER_Regs *timer, Timer_PWMConfig *pwm_config)
{
    // Configure the Pin Control Module (PINCM) with the IOMUX mode.
    IOMUX->SECCFG.PINCM[pwm_config->index] = pwm_config->iomuxMode;

    // Update the Capture/Compare Pin Direction (CCPD) register.
    timer->COMMONREGS.CCPD = pwm_config->ccpd;

    // If the CC register equals 0 or 1, update the arrays associated with
    // the CC0 and CC1. Otherwise, for timers with 4 CC registers, if the
    // CC register equals 2 or 3, update the arrays associated with CC2 and CC3.
    if ((pwm_config->ccr == 0) || (pwm_config->ccr == 1)) {
        timer->COUNTERREGS.CC_01[pwm_config->ccr] = pwm_config->duty;
        timer->COUNTERREGS.CCCTL_01[pwm_config->ccr] = pwm_config->ccctl;
        timer->COUNTERREGS.OCTL_01[pwm_config->ccr] = pwm_config->octl;
        timer->COUNTERREGS.CCACT_01[pwm_config->ccr] = pwm_config->ccact;
    }
    else {
        timer->COUNTERREGS.CC_23[pwm_config->ccr] = pwm_config->duty;
        timer->COUNTERREGS.CCCTL_23[pwm_config->ccr] = pwm_config->ccctl;
        timer->COUNTERREGS.OCTL_23[pwm_config->ccr] = pwm_config->octl;
        timer->COUNTERREGS.CCACT_23[pwm_config->ccr] = pwm_config->ccact;
    }

}

void InitializeTimerCompare(GPTIMER_Regs *timer, Timer_TimerConfig *pwm_config) {

    // Initialize period
    timer->COUNTERREGS.LOAD = pwm_config->period;

    // Initialize timer mode
    timer->COUNTERREGS.CTRCTL = pwm_config->timerMode;
    
}

void TimeDelay(GPTIMER_Regs *timer, Timer_TimerConfig *timer_config)
{
    // Load counter with delay value (in units of clock cycles)
    timer->COUNTERREGS.LOAD = timer_config->period;

    // Set timer mode and enable timer.
    timer->COUNTERREGS.CTRCTL = timer_config->timerMode;
    timer->COUNTERREGS.CTRCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    timer->CPU_INT.ICLR |= GPTIMER_CPU_INT_RIS_Z_SET; // clear event flag

    // Wait until timer counts down to zero.
    while ((timer->CPU_INT.RIS & GPTIMER_CPU_INT_RIS_Z_SET) == 0){};

    // Now, disable the timer.
    timer->COUNTERREGS.CTRCTL &= ~GPTIMER_CCLKCTL_CLKEN_ENABLED;
}

void UpdateDutyCycle(GPTIMER_Regs *timer,Timer_PWMConfig *pwm_config)
{
    if ((pwm_config->ccr == 0) || (pwm_config->ccr == 1)) {
        timer->COUNTERREGS.CC_01[pwm_config->ccr] = pwm_config->duty;
    }
    else {
        timer->COUNTERREGS.CC_23[pwm_config->ccr] = pwm_config->duty;
    }
}

void EnableTimer(GPTIMER_Regs *timer) {
    timer->COUNTERREGS.CTRCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    timer->CPU_INT.ICLR |= GPTIMER_CPU_INT_RIS_Z_SET; // clear event flag
}

void InitializeTimerInterrupt(GPTIMER_Regs *timer, Timer_InterruptConfig *interrupt_config)
{
    // Enable the CPU interrupt(s).
    timer->CPU_INT.IMASK |= interrupt_config->imask;

    __NVIC_EnableIRQ(interrupt_config->IRQnum);
    __NVIC_SetPriority(interrupt_config->IRQnum,interrupt_config->priority);
}
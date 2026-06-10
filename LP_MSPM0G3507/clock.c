#include "clock.h"

//#define PA5INDEX 9
//#define PA6INDEX 10
//#define PA14INDEX 35

// 40 MHz crystal connected to
// PA5 is HFXIN
// PA6 is HFXOUT
// PA14 is CLK_OUT

void ClockInitialization_4MHz_32MHz(uint32_t freq)
{
    uint32_t freq_mask = SYSCTL_SYSOSCCFG_FREQ_SYSOSCBASE;
    uint32_t Mask;
    uint32_t Value;

    if (freq == 4000000) {
        freq_mask = SYSCTL_SYSOSCCFG_FREQ_SYSOSC4M;
    }
    // SYSOSCCFG bits 1,0
    // bits 1,0 set rate of SYSOSC
    //   0 for 32 MHz base oscillator
    //   1 for 4 MHz
    //   2 for user trim
    // bit 10 disable (0 to run, 1 to disable SYSOSC)
    Mask =  SYSCTL_SYSOSCCFG_DISABLE_MASK | SYSCTL_SYSOSCCFG_FREQ_MASK;
    Value = SYSCTL_SYSOSCCFG_DISABLE_DISABLE | freq_mask;
    SYSCTL->SOCLOCK.SYSOSCCFG = (SYSCTL->SOCLOCK.SYSOSCCFG & (~Mask)) | Value; // 4 or 32 MHz

    // MCLKCFG
    //   bits 3-0 MCLK divider is value+1 (1 to 16)
    //   bits 5-4 ULPCLK divider is value+1 (1 to 3)
    //   bit 20 is USELFCLK
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_MDIV_MASK)) |
            ((uint32_t)0x00000000U); // divide by 1
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_UDIV_MASK)) |
            ((uint32_t)0x00000000U); // divide by 1

    // Only one should have been set, but clear both because unknown incoming state
    // Clear SYSOSCCFG.DISABLE to get SYSOSC running again
    // Clear MCLKCFG.USELFCLK to switch MCLK source from LFCLK to SYSOSC
    SYSCTL->SOCLOCK.SYSOSCCFG &= ~SYSCTL_SYSOSCCFG_DISABLE_MASK;    // get SYSOSC running again
    SYSCTL->SOCLOCK.MCLKCFG &= ~SYSCTL_MCLKCFG_USELFCLK_MASK;       // do not use LFCLK

    // MFPCLK is a continuous 4MHz to DAC module in RUN/SLEEP/STOP mode.
    // MFPCLK is produced from SYSOSC
    SYSCTL->SOCLOCK.GENCLKCFG = (SYSCTL->SOCLOCK.GENCLKCFG &
            (~SYSCTL_GENCLKCFG_MFPCLKSRC_MASK)) | SYSCTL_GENCLKCFG_MFPCLKSRC_SYSOSC;
    SYSCTL->SOCLOCK.GENCLKEN = (SYSCTL->SOCLOCK.GENCLKEN &
            ~(SYSCTL_GENCLKEN_MFPCLKEN_MASK)) | SYSCTL_GENCLKEN_MFPCLKEN_ENABLE;

    // Verify SYSOSC -> MCLK
    // bit 17, 0 means MCLK not produced from LFCLK, 1 means MCLK produced from LFCLK
    while((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_CURMCLKSEL_MASK) == SYSCTL_CLKSTATUS_CURMCLKSEL_LFCLK);

    //SCB->SCR System control register, used to control low-power mode behavior
    //   bit 2, 0 is sleep, 1 is deep sleep
    // bit 1, 0 do not sleep when returning to Thread mode, 1 enter sleep/deep sleep on returning from ISR to thread
    SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);   // sleep
    SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk); // no sleep on exit

    SYSCTL->SOCLOCK.BORTHRESHOLD = SYSCTL_BORTHRESHOLD_LEVEL_BORMIN;    // lowest level brown-out reset
}

void ClockInitialization_80MHz(uint32_t enablePA14)
{
    // The following is adapted from page 114 of the Technical Reference Manual (TRM)
    // example of using the PLL.

    uint32_t Mask;
    uint32_t Value;

    if(enablePA14){
        Mask = IOMUX_PINCM_PC_CONNECTED;    // peripheral connected
        Mask |= IOMUX_MODE6;                // I/O function = clock output
        Mask |= IOMUX_PINCM_DRV_DRVVAL1;    // high drive for high-speed I/O
        IOMUX->SECCFG.PINCM[PA14INDEX] = Mask;
        GPIOA->DOESET31_0 = GPIO_DOESET31_0_DIO14_SET; // PA14 output enable
    }

    /* Configure GPIO/IOMUX for HFXT functionality */
    IOMUX->SECCFG.PINCM[PA5INDEX] = 0; // disconnect PA5 from I/O (HFXIN)
    IOMUX->SECCFG.PINCM[PA6INDEX] = 0; // disconnect PA6 from I/O (HFXOUT)
    SYSCTL->SOCLOCK.SYSOSCCFG = (SYSCTL->SOCLOCK.SYSOSCCFG & (~SYSCTL_SYSOSCCFG_FREQ_MASK)) |
            SYSCTL_SYSOSCCFG_FREQ_SYSOSCBASE; // 32 MHz

    /* Check that SYSPLL is disabled before configuration */
    while ((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_SYSPLLOFF_MASK) != SYSCTL_CLKSTATUS_SYSPLLOFF_TRUE){
        __NOP(); // you are here if PLL is already on
    }

    // SYSPLL reference is SYSOSC
    SYSCTL->SOCLOCK.SYSPLLCFG0 = (SYSCTL->SOCLOCK.SYSPLLCFG0 & (~SYSCTL_SYSPLLCFG0_SYSPLLREF_MASK)) |
            SYSCTL_SYSPLLCFG0_SYSPLLREF_SYSOSC;

    // SYSPLLREF (32 MHz) is divided by 2 = 16 MHz
    SYSCTL->SOCLOCK.SYSPLLCFG1 = (SYSCTL->SOCLOCK.SYSPLLCFG1 & (~SYSCTL_SYSPLLCFG1_PDIV_MASK)) |
            SYSCTL_SYSPLLCFG1_PDIV_REFDIV2;

    // Load PLL parameters into SYSPLLPARAM0/1 registers to support f_LOOPIN = 16 MHz.
    // See table 2-6 of TRM.
    SYSCTL->SOCLOCK.SYSPLLPARAM0 = *((volatile uint32_t *) 0x41C4002C);
    SYSCTL->SOCLOCK.SYSPLLPARAM1 = *((volatile uint32_t *) 0x41C40030);

    // Set feedback divider QDIV (multiplies to give output frequency)
    // QDIV = SYSCTL_SYSPLLCFG1_QDIV_QDIVMIN divides by 2, so add offset = 8
    // to obtain desired divisor of QDIV = 10.
    uint32_t QDIV_divisor = (8 << SYSCTL_SYSPLLCFG1_QDIV_OFS) + SYSCTL_SYSPLLCFG1_QDIV_QDIVMIN;
    SYSCTL->SOCLOCK.SYSPLLCFG1 = (SYSCTL->SOCLOCK.SYSPLLCFG1 & (~SYSCTL_SYSPLLCFG1_QDIV_MASK)) | QDIV_divisor;

    // write clock output dividers, enable outputs, and MCLK source to SYSPLLCFG0
    // bits 19-16 RDIVCLK2X   0011 SYSPLLCLK1 is divided by 4
    // bits 15-12 RDIVCLK1    0001 SYSPLLCLK1 is divided by 4
    // bits  11-8 RDIVCLK0    0000 SYSPLLCLK0 is divided by 2
    // bit      6 ENABLECLK2X    1 SYSPLLCLK2X is enabled
    // bit      5 ENABLECLK1     1 SYSPLLCLK1 is enabled
    // bit      4 ENABLECLK0     0 SYSPLLCLK0 is disabled
    // bit      1 MCLK2XVCO      1 SYSPLLCLK2X output is sent to the HSCLK mux
    // bit      0 SYSPLLREF      0 SYSPLL reference is SYSOSC
    Mask = SYSCTL_SYSPLLCFG0_RDIVCLK2X_CLK2XDIV4;
    Mask |= SYSCTL_SYSPLLCFG0_RDIVCLK1_CLK1DIV4;
    Mask |= SYSCTL_SYSPLLCFG0_RDIVCLK0_CLK0DIV2;
    Mask |= SYSCTL_SYSPLLCFG0_ENABLECLK2X_ENABLE;
    Mask |= SYSCTL_SYSPLLCFG0_ENABLECLK1_ENABLE;
    Mask |= SYSCTL_SYSPLLCFG0_ENABLECLK0_DISABLE;
    Mask |= SYSCTL_SYSPLLCFG0_MCLK2XVCO_ENABLE;
    Mask |= SYSCTL_SYSPLLCFG0_SYSPLLREF_SYSOSC;
    SYSCTL->SOCLOCK.SYSPLLCFG0 = Mask; // Mask should equal 0x00031062

    // Enable SYSPLL, Use the HFCLK_IN digital clock input as the HFCLK source
    SYSCTL->SOCLOCK.HSCLKEN = (SYSCTL->SOCLOCK.HSCLKEN & ~(SYSCTL_HSCLKEN_SYSPLLEN_MASK)) |
            SYSCTL_HSCLKEN_SYSPLLEN_ENABLE;

    // Wait until SYSPLL startup is stabilized, bit 9
    while((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_SYSPLLGOOD_MASK) != SYSCTL_CLKSTATUS_SYSPLLGOOD_TRUE) {
        __NOP(); // stuck here means PLL broken
    }

    // HSCLK source is from the SYSPLL
    SYSCTL->SOCLOCK.HSCLKCFG = SYSCTL_HSCLKCFG_HSCLKSEL_SYSPLL;

    // Verify HSCLK source is valid
    while ((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_HSCLKGOOD_MASK) != SYSCTL_CLKSTATUS_HSCLKGOOD_TRUE) {
        __NOP(); // stuck here means HSCLK  broken
    }

    // Switch MCLK to HSCLK, MCLK will use the high speed clock (HSCLK)
    // bit 16 1h = MCLK will use the high speed clock (HSCLK)
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & ~(SYSCTL_MCLKCFG_USEHSCLK_MASK)) |
            SYSCTL_MCLKCFG_USEHSCLK_ENABLE;

    // bit 20 0h = MCLK will not use the low frequency clock (LFCLK)
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_USELFCLK_MASK)) |
            SYSCTL_MCLKCFG_USELFCLK_DISABLE;

    // Verify HSCLK -> MCLK
    while ((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_HSCLKMUX_MASK) != SYSCTL_CLKSTATUS_HSCLKMUX_HSCLK) {
        __NOP();
    }

    // MCLK divide by 1
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_MDIV_MASK));

    // ULPCLK divide by 2
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_UDIV_MASK)) | SYSCTL_MCLKCFG_UDIV_DIVIDE2;

    // 2 flash wait states
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_FLASHWAIT_MASK)) |
            SYSCTL_MCLKCFG_FLASHWAIT_WAIT2;

    // External clock source is PLL with no division.
    Mask = SYSCTL_GENCLKCFG_EXCLKDIVEN_MASK | SYSCTL_GENCLKCFG_EXCLKDIVVAL_MASK | SYSCTL_GENCLKCFG_EXCLKSRC_MASK;
    Value = SYSCTL_GENCLKCFG_EXCLKSRC_SYSPLLOUT1;
    SYSCTL->SOCLOCK.GENCLKCFG = (SYSCTL->SOCLOCK.GENCLKCFG & ~(Mask)) | Value;

    // Enable external clock.
    SYSCTL->SOCLOCK.GENCLKEN |= SYSCTL_GENCLKEN_EXCLKEN_ENABLE;

    //Low Power Mode is configured to be SLEEP0
    SYSCTL->SOCLOCK.BORTHRESHOLD = SYSCTL_BORTHRESHOLD_LEVEL_BORMIN; // lowest level brown-out reset

    //SCB->SCR System control register, used to control low-power mode behavior
    // bit 2, 0 is sleep, 1 is deep sleep
    // bit 1, 0 do not sleep when returning to Thread mode, 1 enter sleep/deep sleep on returning from ISR to thread
    SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);   // sleep
    SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk); // no sleep on exit

    // check HSCLK LFOSC SYSPLL
    Mask = SYSCTL_CLKSTATUS_HSCLKGOOD_MASK | SYSCTL_CLKSTATUS_LFOSCGOOD_MASK | SYSCTL_CLKSTATUS_SYSPLLGOOD_MASK;
    Value = SYSCTL_CLKSTATUS_HSCLKGOOD_TRUE | SYSCTL_CLKSTATUS_LFOSCGOOD_TRUE | SYSCTL_CLKSTATUS_SYSPLLGOOD_TRUE;
    while ((SYSCTL->SOCLOCK.CLKSTATUS & Mask) != Value){
        __NOP();
    }
}

// 40 MHz crystal connected to
//   PA5 is HFXIN
//   PA6 is HFXOUT
void ClockInitialization_40MHz(void)
{
    uint32_t Mask;
    uint32_t Value;

    /* Configure GPIO/IOMUX for HFXT functionality */
    IOMUX->SECCFG.PINCM[PA5INDEX] = 0; // disconnect PA5 from I/O (HFXIN)
    IOMUX->SECCFG.PINCM[PA6INDEX] = 0; // disconnect PA6 from I/O (HFXOUT)
    SYSCTL->SOCLOCK.SYSOSCCFG = (SYSCTL->SOCLOCK.SYSOSCCFG & (~SYSCTL_SYSOSCCFG_FREQ_MASK)) |
            SYSCTL_SYSOSCCFG_FREQ_SYSOSCBASE; // 32 MHz

    // HFCLKCLKCFG
    // bit 28 HFCLK startup
    // bits 13-12 HFXTRSEL
    //     0 = 4 to 8 MHz
    //     1 = 8.01 and 16 MHz
    //     2 =  16.01 and 32 MHz
    //     3 = 32.01 and 48 MHz
    SYSCTL->SOCLOCK.HFCLKCLKCFG = (SYSCTL->SOCLOCK.HFCLKCLKCFG & (~(SYSCTL_HFCLKCLKCFG_HFXTRSEL_MASK))) |
            SYSCTL_HFCLKCLKCFG_HFXTRSEL_RANGE32TO48; // 32.01 and 48 MHz

    // HFXT Startup Time = 0.64 ms: 10*(16.32 ms/255) = 10(64 us) = 0.64 ms, see page 173 TRM
    SYSCTL->SOCLOCK.HFCLKCLKCFG = (SYSCTL->SOCLOCK.HFCLKCLKCFG & (~(SYSCTL_HFCLKCLKCFG_HFXTTIME_MASK))) |
            ((uint32_t)0x0000000aU);

    // HSCLKEN
    //   bit 0 enable
    SYSCTL->SOCLOCK.HSCLKEN = (SYSCTL->SOCLOCK.HSCLKEN & ~(SYSCTL_HSCLKEN_HFXTEN_MASK)) | SYSCTL_HSCLKEN_HFXTEN_ENABLE;

    // enable startup monitor
    SYSCTL->SOCLOCK.HFCLKCLKCFG = (SYSCTL->SOCLOCK.HFCLKCLKCFG & ~(SYSCTL_HFCLKCLKCFG_HFCLKFLTCHK_MASK)) |
            SYSCTL_HFCLKCLKCFG_HFCLKFLTCHK_ENABLE;

    /* Wait until HFXT oscillator is stable. If it does not stabilize, check the hardware/IOMUX settings */
    // CLKSTATUS
    //   bit 21 HSCLKGOOD
    //   bit 8 HFCLKGOOD
    //   bit 4 HSCLKMUX
    while((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_HFCLKGOOD_MASK) != SYSCTL_CLKSTATUS_HFCLKGOOD_TRUE){
        __NOP();
    };

    // HSCLKCFG
    //   bit 0, 1h = HSCLK source is from the HFCLK, 0h = HSCLK source is from the SYSPLL
    SYSCTL->SOCLOCK.HSCLKCFG = SYSCTL_HSCLKCFG_HSCLKSEL_HFCLKCLK;

    // Verify HSCLK source is valid
    while ((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_HSCLKGOOD_MASK) != SYSCTL_CLKSTATUS_HSCLKGOOD_TRUE) {
        __NOP(); // stuck here means HSCLK  broken
    }

    // Switch MCLK to HSCLK, MCLK will use the high speed clock (HSCLK)
    // bit 20 0h = MCLK will not use the low frequency clock (LFCLK)
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_USELFCLK_MASK)) |
            SYSCTL_MCLKCFG_USELFCLK_DISABLE;

    // bit 16 1h = MCLK will use the high speed clock (HSCLK)
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & ~(SYSCTL_MCLKCFG_USEHSCLK_MASK)) |
            SYSCTL_MCLKCFG_USEHSCLK_ENABLE;

    // Verify HSCLK -> MCLK
    while ((SYSCTL->SOCLOCK.CLKSTATUS & SYSCTL_CLKSTATUS_HSCLKMUX_MASK) != SYSCTL_CLKSTATUS_HSCLKMUX_HSCLK) {
        __NOP();
    }

    // MCLKCFG
    //   bits 3-0 MCLK divider is value+1 (1 to 16)
    //   bits 5-4 ULPCLK divider is value+1 (1 to 3)
    // MCLK divide by 1
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_MDIV_MASK));

    // ULPCLK divide by 2
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_UDIV_MASK)) | SYSCTL_MCLKCFG_UDIV_DIVIDE2;

    // 2 flash wait states
    SYSCTL->SOCLOCK.MCLKCFG = (SYSCTL->SOCLOCK.MCLKCFG & (~SYSCTL_MCLKCFG_FLASHWAIT_MASK)) |
            SYSCTL_MCLKCFG_FLASHWAIT_WAIT2;

    // GENCLKCFG
    //   bit 9 is MFCLKSRC =1 for HFCLK, =0 for SYSOSC
    //   bit 7 Clock divider is enabled
    //   bits 6-4 divider = (x+1)*2, x=1 means divide by 4
    //   bits 2-0 EXCLKSRC, 4 = CLK_OUT is HFCLK
    SYSCTL->SOCLOCK.GENCLKCFG = (SYSCTL->SOCLOCK.GENCLKCFG & (~0x00F7)) | 0x94 ; // HFCLK, divide by 4
    SYSCTL->SOCLOCK.GENCLKEN |= 0x01;
    // bit 4 is MFPCLKEN
    // bit 0 is EXCLKEN
    //  MFPCLK is a continuous 4MHz to DAC module in RUN/SLEEP/STOP mode.
    //SYSCTL->SOCLOCK.GENCLKCFG = (SYSCTL->SOCLOCK.GENCLKCFG & (~0x0200)) | 0x00; // MFPCLK is sourced from SYSOSC
    //SYSCTL->SOCLOCK.GENCLKEN |= SYSCTL_GENCLKEN_MFPCLKEN_ENABLE;

    //Low Power Mode is configured to be SLEEP0
    SYSCTL->SOCLOCK.BORTHRESHOLD = SYSCTL_BORTHRESHOLD_LEVEL_BORMIN; // lowest level brown-out reset

    // Wait for these clocks to stabilize
    // Bit 21 HSCLKGOOD
    // Bit 11 LFOSCGOOD
    // Bit 8 HFCLKGOOD
    Mask = SYSCTL_CLKSTATUS_HSCLKGOOD_MASK | SYSCTL_CLKSTATUS_LFOSCGOOD_MASK | SYSCTL_CLKSTATUS_HFCLKGOOD_MASK;
    Value = SYSCTL_CLKSTATUS_HSCLKGOOD_TRUE | SYSCTL_CLKSTATUS_LFOSCGOOD_TRUE | SYSCTL_CLKSTATUS_HFCLKGOOD_TRUE;
    while ((SYSCTL->SOCLOCK.CLKSTATUS & Mask) != Value){
        __NOP();
    }

    //SCB->SCR System control register, used to control low-power mode behavior
    // bit 2, 0 is sleep, 1 is deep sleep
    // bit 1, 0 do not sleep when returning to Thread mode, 1 enter sleep/deep sleep on returning from ISR to thread
    SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);   // sleep
    SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk); // no sleep on exit

}

// Valvano's version of this function.
//void Clock_Delay(uint32_t cycles){
//    /* There will be a 2 cycle delay here to fetch & decode instructions
//     * if branch and linking to this function */
//
//    /* Subtract 2 net cycles for constant offset: +2 cycles for entry jump,
//     * +2 cycles for exit, -1 cycle for a shorter loop cycle on the last loop,
//     * -1 for this instruction */
//#ifdef __GNUC__
//    __asm(".syntax unified");
//#endif
//    __asm volatile(
//            "            SUBS  R0, R0, #2; \n"
//            "Clock_Loop: SUBS  R0, R0, #4; \n" // C=1 if no overflow
//            "            NOP;              \n" // C=0 when R0 passes through 0
//            "            BHS   Clock_Loop; \n"
//            /* Return: 2 cycles */
//    );
//}

__attribute__((noinline)) void ClockDelay(uint32_t cycles)
{
    // Note that optimization must be "turned off". Otherwise, R0 is not
    // necessarily assigned the parameter, cycles, in which case whatever
    // value is in R0 determines how long this function takes to execute.

    /* Based on function in driver library (see dl_common.c).
     * Time delay based on number of clock cycles. Based on function
     * in driver library (see dl_common.c).
     *
     * Input: cycles (in R0) contains number of clock cycles to delay
     *
     * Each loop iteration is 4 clock cycles. However, last
     * iteration is 3 clock cycles because branch is not taken.
     * Finally, calling and returning from this function each require
     * 2 clock cycles, and the first SUBS requires 1 clock cycle.
     *
     * So, cycles = 4*n + 2 (call) + 2 (return) + 1 (SUBS) - 3 (last iteration).
     * Or, n = (cycles - 2)/4. So, first SUBS instruction adjusts R0 accordingly,
     * where n is the number of iterations of the loop.
     */
    __asm volatile (
            "               SUBS    R0, R0, #2;   \n"
            "Clock_loop:    SUBS    R0, R0, #4;   \n" // C=1 if no overflow
            "               NOP;                  \n" // C=0 when R0 passes through 0
            "               BHS Clock_loop;       \n" // branches back to Clock_loop if R0 >= 0
    );
}

// Approximate delay using BusFreq clock
void ClockDelay_1ms(uint32_t ms)
{
    uint32_t delay = ms; // in milliseconds

    // Each call to ClockDelay is count cycles
    uint32_t count = ONE_MILLISECOND_DELAY;
    while (delay) {
        ClockDelay(count);
        delay--;
    }
}


// initialize 24 bit timer
void SysTick_Init(void)
{
    SysTick->CTRL = 0;           // 1) disable SysTick during setup
    SysTick->LOAD = 0;           // 2) stops
    SysTick->VAL = 0;            // 3) any write to current clears it
    SysTick->CTRL = 0x00000005;  // 4) enable SysTick with core clock
}

// The delay parameter is in units of the core clock.
// about a 50 cycle overhead
void SysTick_Wait(uint32_t delay)
{
    SysTick->LOAD = delay-1;  // number of counts to wait
    SysTick->VAL = 0;         // any value written to VAL clears
    while((SysTick->CTRL&0x00010000)==0){} // wait for count flag
}

// The t parameter is in units 10ms.
void SysTick_Wait10ms(uint32_t t)
{
    for(; t; t--){
        SysTick_Wait(800000);  // wait 10ms, assuming 80 MHz clock
    }
}




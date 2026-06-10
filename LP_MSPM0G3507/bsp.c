#include "bsp.h"

// Valid clock frequencies are 4 MHz, 32 MHz, 40 MHz, and 80 MHz
uint32_t ClockFrequency __attribute__((weak)) = 32000000; // 32 MHz

void Error_Handler(void)
{
    __disable_irq();

    while (1) {
        __NOP();
    }
}

uint32_t CPUCyclesPer1us(void)
{
    uint32_t ReturnValue = 0;

    switch (ClockFrequency) {
    case 4000000: ReturnValue = 4; break;
    case 40000000: ReturnValue = 40; break;
    case 80000000: ReturnValue = 80; break;
    default: ReturnValue = 32;
    }

    return ReturnValue;
}

uint32_t CPUCyclesPer1ms(void)
{
    uint32_t ReturnValue = 1000*CPUCyclesPer1us();

    return ReturnValue;
}

void ActivatePortAandB(void)
{
    // Reset ports A and B, and then enable power.
    /*** Reset must occur before enabling power ***/

    GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
    GPIOA->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);

    GPIOB->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
    GPIOB->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);

    // Wait for power to stabilize.
    ClockDelay(POWER_STARTUP_DELAY);
}

void InitializeLaunchpad(uint32_t CLOCK_FREQUENCY)
{
    ActivatePortAandB();

    // IOMUX configuration   
    // Red LED
    IOMUX->SECCFG.PINCM[RED_LED_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE1);

    // RGB LEDs
    IOMUX->SECCFG.PINCM[RGB_RED_LED_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE1);
    IOMUX->SECCFG.PINCM[RGB_GREEN_LED_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE1);
    IOMUX->SECCFG.PINCM[RGB_BLUE_LED_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE1);

    // Push buttons
    IOMUX->SECCFG.PINCM[S1_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM_INENA_ENABLE | IOMUX_PINCM_PIPD_ENABLE | IOMUX_MODE1);
    IOMUX->SECCFG.PINCM[S2_INDEX] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM_INENA_ENABLE | IOMUX_PINCM_PIPU_ENABLE | IOMUX_MODE1);

    // Enable outputs in the Data Output Enable (DOE) register.
    RED_LED_PORT->DOE31_0 |= RED_LED_MASK;
    RGB_RED_LED_PORT->DOE31_0 |= RGB_RED_LED_MASK;
    RGB_GREEN_LED_PORT->DOE31_0 |= RGB_GREEN_LED_MASK;
    RGB_BLUE_LED_PORT->DOE31_0 |= RGB_BLUE_LED_MASK;

    // Turn off LEDs initially.
    TurnOffLED1();
    TurnOffLED2Red();
    TurnOffLED2Green();
    TurnOffLED2Blue();

    // Finally, initialize the clocking system.
    switch (CLOCK_FREQUENCY) {
        case 40000000: ClockInitialization_40MHz();
        case 80000000: ClockInitialization_80MHz(0); // disable clk_out
        default: ClockInitialization_4MHz_32MHz(CLOCK_FREQUENCY);
    }
}

uint32_t ReadS1(void)
{
    uint32_t ReturnValue = 0;

    ReturnValue = (uint32_t) ((S1_PORT->DIN31_0 & S1_MASK) >> S1_PIN);

    return ReturnValue;
}

uint32_t ReadS2(void)
{
    uint32_t ReturnValue = 0;

    ReturnValue = (uint32_t) ((S2_PORT->DIN31_0 & S2_MASK) >> S2_PIN);

    return !ReturnValue;
}

void TurnOnLED1(void)
{
    RED_LED_PORT->DOUT31_0 &= ~RED_LED_MASK;
}

void TurnOffLED1(void)
{
    RED_LED_PORT->DOUT31_0 |= RED_LED_MASK;
}

void ToggleLED1(void)
{
    RED_LED_PORT->DOUT31_0 ^= RED_LED_MASK;
}

void TurnOnLED2Red(void)
{
    RGB_RED_LED_PORT->DOUT31_0 |= RGB_RED_LED_MASK;
}

void TurnOffLED2Red(void)
{
    RGB_RED_LED_PORT->DOUT31_0 &= ~RGB_RED_LED_MASK;
}

void ToggleLED2Red(void)
{
    RGB_RED_LED_PORT->DOUT31_0 ^= RGB_RED_LED_MASK;
}

void TurnOnLED2Green(void)
{
    RGB_GREEN_LED_PORT->DOUT31_0 |= RGB_GREEN_LED_MASK;
}

void TurnOffLED2Green(void)
{
    RGB_GREEN_LED_PORT->DOUT31_0 &= ~RGB_GREEN_LED_MASK;
}

void ToggleLED2Green(void)
{
    RGB_GREEN_LED_PORT->DOUT31_0 ^= RGB_GREEN_LED_MASK;
}

void TurnOnLED2Blue(void)
{
    RGB_BLUE_LED_PORT->DOUT31_0 |= RGB_BLUE_LED_MASK;
}

void TurnOffLED2Blue(void)
{
    RGB_BLUE_LED_PORT->DOUT31_0 &= ~RGB_BLUE_LED_MASK;
}

void ToggleLED2Blue(void)
{
    RGB_BLUE_LED_PORT->DOUT31_0 ^= RGB_BLUE_LED_MASK;
}

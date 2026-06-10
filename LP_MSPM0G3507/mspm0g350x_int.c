#include "mspm0g350x_int.h"

// Declaration of external function for default IRQ handler.
extern void Default_Handler(void);

// Default callback function for use with weak declarations of
// callback structures.
void Default_Callback(void *CallbackObject)
{
    Default_Handler();
}

/*
 * Weak declarations initialized to default callback function.
 */
Callback_s Group1 __attribute__((weak)) = {(void *) 0,Default_Callback};

Callback_s TimerG8 __attribute__((weak)) = {(void *) 0,Default_Callback};


/* Device Specific Interrupt Handlers */
void GROUP0_IRQHandler(void)
{
    Default_Handler();
}
void GROUP1_IRQHandler(void)
{
    Group1.CallbackFunc(Group1.CallbackObject);
}
void TIMG8_IRQHandler(void)
{
    TimerG8.CallbackFunc(TimerG8.CallbackObject);
}
void UART3_IRQHandler(void)
{
    Default_Handler();
}
void ADC0_IRQHandler(void)
{
    Default_Handler();
}
void ADC1_IRQHandler(void)
{
    Default_Handler();
}
void CANFD0_IRQHandler(void)
{
    Default_Handler();
}
void DAC0_IRQHandler(void)
{
    Default_Handler();
}
void SPI0_IRQHandler(void)
{
    Default_Handler();
}
void SPI1_IRQHandler(void)
{
    Default_Handler();
}
void UART1_IRQHandler(void)
{
    Default_Handler();
}
void UART2_IRQHandler(void)
{
    Default_Handler();
}
void UART0_IRQHandler(void)
{
    Default_Handler();
}
void TIMG0_IRQHandler(void)
{
    Default_Handler();
}
void TIMG6_IRQHandler(void)
{
    Default_Handler();
}
void TIMA0_IRQHandler(void)
{
    Default_Handler();
}
void TIMA1_IRQHandler(void)
{
    Default_Handler();
}
void TIMG7_IRQHandler(void)
{
    Default_Handler();
}
void TIMG12_IRQHandler(void)
{
    Default_Handler();
}
void I2C0_IRQHandler(void)
{
    Default_Handler();
}
void I2C1_IRQHandler(void)
{
    Default_Handler();
}
void AES_IRQHandler(void)
{
    Default_Handler();
}
void RTC_IRQHandler(void)
{
    Default_Handler();
}
void DMA_IRQHandler(void)
{
    Default_Handler();
}

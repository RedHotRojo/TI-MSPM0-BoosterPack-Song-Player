#include "uart.h"

void InitializeUART(UART_Regs *uart, UART_Config *uart_config)
{
    uint32_t Mask;

    uart->GPRCM.RSTCTL = (UART_RSTCTL_KEY_UNLOCK_W | UART_RSTCTL_RESETASSERT_ASSERT | UART_RSTCTL_RESETSTKYCLR_CLR);
    uart->GPRCM.PWREN = (UART_PWREN_KEY_UNLOCK_W | UART_PWREN_ENABLE_ENABLE);

    ClockDelay(POWER_STARTUP_DELAY); // time for uart to power up

    // Configure port pins for alternate UART function
    Mask = IOMUX_PINCM_PC_CONNECTED;    // peripheral connected.
    Mask |= uart_config->Tx_iomuxMode;  // UART Tx
    IOMUX->SECCFG.PINCM[uart_config->Tx_index]  = Mask;

    Mask = IOMUX_PINCM_PC_CONNECTED;    // peripheral connected.
    Mask |= uart_config->Rx_iomuxMode;  // UART Rx
    Mask |= IOMUX_PINCM_INENA_ENABLE;   // enable as input
    IOMUX->SECCFG.PINCM[uart_config->Rx_index]  = Mask;

    // Clock configuration.
    uart->CLKSEL = UART_CLKSEL_BUSCLK_SEL_ENABLE;
    uart->CLKDIV = UART_CLKDIV_RATIO_DIV_BY_1;

    Mask = UART_CTL0_ENABLE_DISABLE;
    Mask |= UART_CTL0_LBE_DISABLE;      // no loop back
    Mask |= UART_CTL0_RXE_ENABLE;
    Mask |= UART_CTL0_TXE_ENABLE;
    Mask |= UART_CTL0_RTS_CLR;          // not RTS
    Mask |= UART_CTL0_RTSEN_DISABLE;    // no RTS hardware
    Mask |= UART_CTL0_CTSEN_DISABLE;    // no CTS hardware
    Mask |= UART_CTL0_MODE_UART;        // normal mode
    Mask |= UART_CTL0_FEN_ENABLE;       // enable FIFO
    Mask |= UART_CTL0_HSE_OVS16;        // 16x oversampling
    uart->CTL0 = Mask;

    if (uart_config->ClockFrequency == 40000000){
        // 20000000/16 = 1,250,000 Hz
        // Baud = 115200
        //   1,250,000/115200 = 10.850694
        //   divider = 10+(54/64) = 10.84375
        uart->IBRD = 10;
        uart->FBRD = 54; // baud = 1,250,000/10.84375 = 115,274
    }
    else if (uart_config->ClockFrequency == 32000000){
        // 32000000/16 = 2,000,000 Hz
        // Baud = 115200
        //   2,000,000/115200 = 17.361
        //   divider = 17+(23/64) = 17.359
        uart->IBRD = 17;
        uart->FBRD = 23; // baud = 2,000,000/17.359 = 115,211
    }
    else if (uart_config->ClockFrequency == 80000000){
        // 40000000/16 = 2,500,000 Hz
        // Baud = 115200
        //    2,500,000/115200 = 21.701388
        //   divider = 21+(45/64) = 21.703125
        uart->IBRD = 21;
        uart->FBRD = 45; // baud = 2,500,000/21.703125 = 115,191
    }
    else
        Error_Handler();

    Mask = UART_LCRH_BRK_DISABLE;       // no break
    Mask |= UART_LCRH_PEN_DISABLE;      // no parity
    Mask |= UART_LCRH_EPS_ODD;          // parity select
    Mask |= UART_LCRH_STP2_DISABLE;     // 1 stop bit
    Mask |= UART_LCRH_WLEN_DATABIT8;    // 8 data bits
    uart->LCRH = Mask;

    uart->CTL0 |= UART_CTL0_ENABLE_ENABLE; // enable UART
}

void PrintOutputBuffer(UART_Regs *uart, uint8_t *OutputBuffer)
{
    // Note that this function assumes that the last character
    // in the string contained in OutputBuffer is the NULL character.
    for (int i = 0; OutputBuffer[i]; i++) {
        while(uart->STAT&1);
        uart->TXDATA = OutputBuffer[i];
    }
    return;
}

uint8_t GetUserInput(UART_Regs *uart)
{
    uint8_t ReturnValue;

    while ((uart->STAT&0x04) == 0x04);

    ReturnValue = (char)(uart->RXDATA);

    return ReturnValue;
}

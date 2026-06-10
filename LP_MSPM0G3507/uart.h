#ifndef UART_H_
#define UART_H_

#include <ti/devices/msp/msp.h>
#include "bsp.h"

/*!
 * @brief Configuration struct for UART configuration.
 */
typedef struct {
    uint8_t Tx_index;          // Index for pin control management register
    uint32_t Tx_iomuxMode;
    uint8_t Rx_index;         // Index for pin control management register
    uint32_t Rx_iomuxMode;
    uint32_t ClockFrequency;
} UART_Config;

void InitializeUART(UART_Regs *uart, UART_Config *uart_config);

// Transmits all of the characters in OutputBuffer.
// Busy-wait synchronization is used for transmission.
void PrintOutputBuffer(UART_Regs *uart, uint8_t *OutputBuffer);

// Receives input from the keyboard and returns the received character.
// Busy-wait synchronization is used for reception.
uint8_t GetUserInput(UART_Regs *uart);

#endif /* UART_H_ */

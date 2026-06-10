#ifndef SPI_H_
#define SPI_H_

#include <ti/devices/msp/msp.h>
#include "bsp.h"

/*!
 * @brief Configuration struct for SPI configuration.
 */
typedef struct {
    uint8_t SCK_index;          // Index for pin control management register
    uint32_t SCK_iomuxMode;
    uint8_t PICO_index;         // Index for pin control management register
    uint32_t PICO_iomuxMode;
    uint8_t POCI_index;         // Index for pin control management register
    uint32_t POCI_iomuxMode;
    uint8_t CS_index;         // Index for pin control management register
    uint32_t CS_iomuxMode;
    uint32_t divideRatio;       // Clock divider
    uint32_t scr;
    uint32_t ctl0;
    uint32_t ctl1;
    uint32_t ClockFrequency;
} SPI_Config;


void InitializeSPI(SPI_Regs *spi, SPI_Config *spi_config);
void SPISendByte(SPI_Regs *spi, uint8_t SendData);
uint8_t SPIReceiveByte(SPI_Regs *spi);

#endif /* SPI_H_ */

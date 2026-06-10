#include "SPI.h"

void InitializeSPI(SPI_Regs *spi, SPI_Config *spi_config)
{
    uint32_t Mask = 0;

    // RSTCLR to SPI1 peripherals
    Mask = SPI_RSTCTL_KEY_UNLOCK_W;         //   bits 31-24 unlock key 0xB1
    Mask |= SPI_RSTCTL_RESETSTKYCLR_CLR;    //   bit 1 is Clear reset sticky bit
    Mask |= SPI_RSTCTL_RESETASSERT_ASSERT;  //   bit 0 is reset gpio port
    spi->GPRCM.RSTCTL = Mask;

    // Enable power to SPI peripherals
    Mask = SPI_PWREN_KEY_UNLOCK_W;          //   bits 31-24 unlock key 0x26
    Mask |= SPI_PWREN_ENABLE_ENABLE;        //   bit 0 is Enable Power
    spi->GPRCM.PWREN = Mask;

    // Configure SPI SCLK, MOSI, and MISO IOMUX functions.
    IOMUX->SECCFG.PINCM[spi_config->SCK_index]  = spi_config->SCK_iomuxMode;
    IOMUX->SECCFG.PINCM[spi_config->PICO_index]  = spi_config->PICO_iomuxMode;
    IOMUX->SECCFG.PINCM[spi_config->POCI_index]  = spi_config->POCI_iomuxMode;
    IOMUX->SECCFG.PINCM[spi_config->CS_index]  = spi_config->CS_iomuxMode;

    ClockDelay(POWER_STARTUP_DELAY);

    spi->CLKSEL = SPI_CLKSEL_SYSCLK_SEL_ENABLE; // bit 3 SYSCLK

    spi->CLKDIV = spi_config->divideRatio;

    // bits 2-0 n (0 to 7), divide by n+1
    // Set the bit rate clock divider to generate the serial output clock
    //     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
    spi->CLKCTL = (spi_config->ClockFrequency/(spi_config->divideRatio+1))/((1+spi_config->scr)*2);

    // bit 14 CSCLR=0 not cleared
    // bits 13-12 CSSEL=0 CS0
    // bit 9 SPH = 0
    // bit 8 SPO = 0
    // bits 6-5 FRF = 00 (3 wire)
    // bits 4-0 n=7, data size is n+1 (8bit data)
    spi->CTL0 = spi_config->ctl0;

    // bits 29-24 RXTIMEOUT=0
    // bits 23-16 REPEATX=0 disabled
    // bits 15-12 CDMODE=0 manual
    // bit 11 CDENABLE=0 CS3
    // bit 7-5 =0 no parity
    // bit 4=1 MSB first
    // bit 3=0 POD (not used, not peripheral)
    // bit 2=1 CP controller mode
    // bit 1=0 LBM disable loop back
    // bit 0=1 enable SPI
    spi->CTL1 = spi_config->ctl1;

    // Rx event as soon as 1 byte received.
    spi->IFLS |= SPI_IFLS_RXIFLSEL_LVL_1_4;
}

void SPISendByte(SPI_Regs *spi, uint8_t SendData)
{
    // Wait while transmit buffer is full
    while ((spi->STAT & SPI_STAT_TNF_MASK) == SPI_STAT_TNF_FULL);

    // Transmit the data.
    spi->TXDATA = SendData;
}

uint8_t SPIReceiveByte(SPI_Regs *spi)
{
    uint8_t ReturnValue = 0;

    // "Drain" Rx FIFO to ensure its empty (because there may be "garbage" data in the Rx FIFO).
    while (!((spi->STAT & SPI_STAT_RFE_MASK) == SPI_STAT_RFE_EMPTY)){ReturnValue = spi->RXDATA;}

    // Transmit dummy data to generate clock
    SPISendByte(spi,0xFF);

    // Wait until receive event.
    while ((spi->STAT & SPI_STAT_RFE_MASK) == SPI_STAT_RFE_EMPTY);

    ReturnValue = spi->RXDATA;

    return ReturnValue;
}

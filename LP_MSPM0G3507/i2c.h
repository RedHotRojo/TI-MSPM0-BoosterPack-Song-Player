#ifndef I2C_H_
#define I2C_H_

#include <ti/devices/msp/msp.h>
#include "bsp.h"

/*!
 * @brief Configuration struct for I2C configuration.
 */
typedef struct {
    uint8_t SCL_index;          // Index for pin control management register
    uint8_t SDA_index;         // Index for pin control management register
    uint32_t ClockFrequency;
} I2C_Config;

/*
 * @brief Initialize an I2C hardware module.
 * @param i2c: Pointer to I2C_Regs structure.
 * @param i2c_config: Pointer to I2C_Config structure.
 * @return none
 */
void InitializeI2C(I2C_Regs *i2c, I2C_Config *i2c_config);

/*
 * @brief Send information to I2C device.
 * @param i2c: Pointer to I2C_Regs structure.
 * @param i2c_address: I2C address of target device.
 * @param buffer: Array of bytes to send to target device.
 * @param count: Number of bytes to send.
 * @return Boolean value indicating whether transmission was successful.
 */
uint16_t I2C_Send(I2C_Regs *i2c, uint8_t i2c_address, uint8_t *buffer, uint16_t count);

/*
 * @brief Receive information from I2C device.
 * @param i2c: Pointer to I2C_Regs structure.
 * @param i2c_address: I2C address of target device.
 * @param buffer: Array to store received bytes from target device.
 * @param count: Number of bytes received.
 * @return none.
 */
void I2C_Receive(I2C_Regs *i2c, uint8_t i2c_address, uint8_t *buffer, uint16_t count);

#endif /* I2C_H_ */

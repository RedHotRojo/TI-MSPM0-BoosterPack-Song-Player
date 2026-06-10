#include "opt3001.h"

void InitializeOpt3001Sensor(I2C_Regs *i2c)
{
    uint8_t buffer[3] = {CONFIGURATION_REGISTER_ADDRESS,0,0};
    uint16_t ConfigurationValue = 0;

    // First, configure the sensor.
    ConfigurationValue = (RANGE_NUMBER(12) + LATCH + MODE_CONVERSION_EQUALS_3);

    // Now, create a buffer of bytes to send to the sensor.
    buffer[1] = (uint8_t) ((ConfigurationValue >> 8) & 0xFF);
    buffer[2] = (uint8_t) (ConfigurationValue & 0xFF);
    I2C_Send(i2c,OPT3001_ADDRESS, (uint8_t *) buffer,3);
}

uint16_t sensorOpt3001Read(I2C_Regs *i2c,uint8_t SensorRegisterAddress)
{
    uint16_t SensorReturnValue = 0;

    uint8_t buffer[2];
    for (int i = 0; i < 2; i++) buffer[i] = 0;

    // Use I2C to send the address of the register to read.
    I2C_Send(i2c, OPT3001_ADDRESS, &SensorRegisterAddress, 1);

    // Then use I2C to receive the register contents as 2 bytes.
    I2C_Receive(i2c, OPT3001_ADDRESS, (uint8_t *) buffer, 2);

    // Finally, assign the 2 bytes to the return value as a 16-bit unsigned integer
    SensorReturnValue = (((uint16_t) buffer[0]) << 8) | ((uint16_t) buffer[1]);

    return SensorReturnValue;
}

float sensorOpt3001Convert(uint16_t rawData)
{
    float convertedLux = 0.0;

    // Use the transfer function equation from the OPT3001 datasheet to convert
    // the sensor reading to an equivalent value in units of lux.

    uint8_t E = (rawData >> 12) & 0x0F;

    uint16_t R = rawData & 0x0FFF;

    float LSB_size = 0.01 * (1 << E);

    convertedLux = LSB_size * R;

    return convertedLux;
}

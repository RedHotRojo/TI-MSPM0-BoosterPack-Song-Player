#ifndef OPT3001_H_
#define OPT3001_H_

#include <ti/devices/msp/msp.h>
#include "i2c.h"

#define OPT3001_INT_PIN  27
#define OPT3001_INT_MASK ((uint32_t) (0x00000001U << OPT3001_INT_PIN))
#define OPT3001_INT_PORT GPIOA
#define OPT3001_INT_INDEX PA27INDEX

// Opt3001 sensor I2C address
#define OPT3001_ADDRESS 0x44

// Opt3001 register addresses
#define RESULT_REGISTER_ADDRESS             0x00
#define CONFIGURATION_REGISTER_ADDRESS      0x01
#define LOW_LIMIT_REGISTER_ADDRESS          0x02
#define HIGH_LIMIT_REGISTER_ADDRESS         0x03
#define MANUFACTURER_ID_REGISTER_ADDRESS    0x7E
#define DEVICE_ID_REGISTER_ADDRESS          0x7F

// Opt3001 configuration register fields
#define FAULT_COUNT_EQUALS_1                (uint16_t) 0x0000
#define FAULT_COUNT_EQUALS_2                (uint16_t) 0x0001
#define FAULT_COUNT_EQUALS_3                (uint16_t) 0x0002
#define FAULT_COUNT_EQUALS_4                (uint16_t) 0x0003
#define MASK_EXPONENT                       (uint16_t) 0x0004
#define POLARITY                            (uint16_t) 0x0008
#define LATCH                               (uint16_t) 0x0010
#define MODE_CONVERSION_EQUALS_0            (uint16_t) (0*(0x0200))
#define MODE_CONVERSION_EQUALS_1            (uint16_t) (1*(0x0200))
#define MODE_CONVERSION_EQUALS_2            (uint16_t) (2*(0x0200))
#define MODE_CONVERSION_EQUALS_3            (uint16_t) (3*(0x0200))
#define CONVERSION_TIME                     (uint16_t) 0x0800
#define RANGE_NUMBER(VALUE)                 (uint16_t) (VALUE*(0x1000))

// Function prototypes
void InitializeOpt3001Sensor(I2C_Regs *i2c);
uint16_t sensorOpt3001Read(I2C_Regs *i2c,uint8_t SensorRegisterAddress);
float sensorOpt3001Convert(uint16_t rawData);

#endif

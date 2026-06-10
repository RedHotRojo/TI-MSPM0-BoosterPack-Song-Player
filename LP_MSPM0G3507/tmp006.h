#ifndef TMP006_H_
#define TMP006_H_

#include <ti/devices/msp/msp.h>
#include "i2c.h"
#include <math.h>

#define TMP006_INT_PIN  16
#define TMP006_INT_MASK ((uint32_t) (0x00000001U << TMP006_INT_PIN))
#define TMP006_INT_PORT GPIOB
#define TMP006_INT_INDEX PB16INDEX

// TMP006 sensor I2C address
#define TMP006_ADDRESS 0x40

// TMP006 register addresses
#define VOLTAGE_REGISTER_ADDRESS        0x00
#define LOCAL_TEMP_REGISTER_ADDRESS     0x01
#define TMP006_CONFIG_REGISTER_ADDRESS  0x02
#define TMP006_MAN_ID_REGISTER_ADDRESS  0xFE
#define TMP006_DEV_ID_REGISTER_ADDRESS  0xFF

// TMP006 configuration register fields
#define RST             (uint16_t) 0x8000
#define MOD             (uint16_t) 0x7000
#define CR_MASK         (uint16_t) 0x0E00
#define CR_SAMPLES_1    (uint16_t) 0x0000
#define CR_SAMPLES_2    (uint16_t) 0x0200
#define CR_SAMPLES_4    (uint16_t) 0x0400
#define CR_SAMPLES_8    (uint16_t) 0x0600
#define CR_SAMPLES_16   (uint16_t) 0x0800
#define DRDY_ENABLE     (uint16_t) 0x0100
#define DDRY_READY      (uint16_t) 0x0080

// From the Adafruit library for this part
#define TMP006_B0 -0.0000294
#define TMP006_B1 -0.00000057
#define TMP006_B2 0.00000000463
#define TMP006_C2 13.4
#define TMP006_TREF 298.15
#define TMP006_A2 -0.00001678
#define TMP006_A1 0.00175
#define TMP006_S0 6.4  // * 10^-14

// Function prototypes
void InitializeTmp006Sensor(I2C_Regs *i2c);
uint16_t sensorTmp006Read(I2C_Regs *i2c,uint8_t SensorRegisterAddress);
float sensorTmp006ConvertTemp(int16_t rawV, int16_t rawT);

#endif /* TMP006_H_ */

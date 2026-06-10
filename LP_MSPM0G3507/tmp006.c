#include "tmp006.h"

void InitializeTmp006Sensor(I2C_Regs *i2c)
{
    // Configure the TMP006 interrupt input to use
    // when polling for conversion completion.
    IOMUX->SECCFG.PINCM[TMP006_INT_INDEX] =
            (IOMUX_PINCM_INENA_ENABLE | IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE1);
}

uint16_t sensorTmp006Read(I2C_Regs *i2c,uint8_t SensorRegisterAddress)
{
    uint8_t buffer[3] = {TMP006_CONFIG_REGISTER_ADDRESS,0,0};
    uint16_t ConfigurationValue = (MOD | CR_SAMPLES_4 | DRDY_ENABLE);
    uint8_t RegisterAddress = SensorRegisterAddress;
    uint16_t SensorReturnValue = 0;
    uint32_t i;

    // Initiate a conversion.
    buffer[1] = (uint8_t) ((ConfigurationValue >> 8) & 0xFF);
    buffer[2] = (uint8_t) (ConfigurationValue & 0xFF);
    I2C_Send(i2c,TMP006_ADDRESS, (uint8_t *) buffer,3);

    // Wait for conversion to complete.
    while ((TMP006_INT_PORT->DIN31_0 & TMP006_INT_MASK) == TMP006_INT_MASK);

    // Read desired sensor data.
    for (i = 0; i < 3; i++) buffer[i] = 0;
    I2C_Send(i2c,TMP006_ADDRESS,&RegisterAddress,1);
    I2C_Receive(i2c,TMP006_ADDRESS,(uint8_t *) buffer,2);
    SensorReturnValue = (((uint16_t) buffer[0]) << 8) | ((uint16_t) buffer[1]);

    return SensorReturnValue;
}

float sensorTmp006ConvertTemp(int16_t rawV, int16_t rawT)
{
    double Vobj = (double) rawV;
    double Tdie = (double) (rawT >> 2); // see page 15 of datasheet
    double ReturnValue = 0;

    // Voltage LSB = 156.25 nV for 16 bits (2's complement)
    // (full scale = +5.12 mV to -5.12 mV)
    Vobj = Vobj * 156.25; // nV
    Vobj = Vobj/1000.0;   // uV
    Vobj = Vobj/1000.0;   // mV
    Vobj = Vobj/1000.0;   // V

    Tdie = Tdie * 0.03125; // convert to C
    Tdie = Tdie + 273.15; // convert to Kelvin

    // Temperature LSB = 0.03125 C for 16 bits (2's complement)
    // (full scale = -55 C to 150 C)
    double tdie_tref = Tdie - TMP006_TREF;
    double S = (1 + TMP006_A1*tdie_tref + TMP006_A2*tdie_tref*tdie_tref);
    S *= TMP006_S0;
    S /= 10000000;
    S /= 10000000;

    double Vos = TMP006_B0 + TMP006_B1*tdie_tref + TMP006_B2*tdie_tref*tdie_tref;
    double fVobj = (Vobj - Vos) + TMP006_C2*(Vobj-Vos)*(Vobj-Vos);
    double Tobj = sqrt(sqrt(Tdie * Tdie * Tdie * Tdie + fVobj/S));

    Tobj -= 273.15; // Kelvin -> C

    return (float) Tobj;
}

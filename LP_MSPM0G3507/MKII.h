#ifndef MKII_H_
#define MKII_H_

#include "bsp.h"

// Complete the following #define statements

// Push Button S1 J4.33
#define BP_S1_INDEX PA11INDEX
#define BP_S1_PIN 11
#define BP_S1_MASK ((uint32_t) (0x00000001U << BP_S1_PIN))
#define BP_S1_PORT GPIOA

#define BP_S1_POL_RISING ((uint32_t) (GPIO_POLARITY_15_0(BP_S1_PIN, GPIO_POL_RISING)))
#define BP_S1_POL_FALLING ((uint32_t) (GPIO_POLARITY_15_0(BP_S1_PIN, GPIO_POL_FALLING)))
#define BP_S1_INTERRUPT GPIOA_INT_IRQn

// Push Button S2 J4.32
#define BP_S2_INDEX PA12INDEX
#define BP_S2_PIN 12
#define BP_S2_MASK ((uint32_t) (0x00000001U << BP_S2_PIN))
#define BP_S2_PORT GPIOA

#define BP_S2_POL_RISING ((uint32_t) (GPIO_POLARITY_15_0(BP_S2_PIN, GPIO_POL_RISING)))
#define BP_S2_POL_FALLING ((uint32_t) (GPIO_POLARITY_15_0(BP_S2_PIN, GPIO_POL_FALLING)))
#define BP_S2_INTERRUPT GPIOA_INT_IRQn

// LED Red J4.39
#define BP_RGB_R_PIN 1
#define BP_RGB_R_MASK ((uint32_t) (0x00000001U << BP_RGB_R_PIN))
#define BP_RGB_R_PORT GPIOB
#define BP_RGB_R_INDEX PB1INDEX

// LED Green J4.38
#define BP_RGB_G_PIN 28
#define BP_RGB_G_MASK ((uint32_t) (0x00000001U << BP_RGB_G_PIN))
#define BP_RGB_G_PORT GPIOA
#define BP_RGB_G_INDEX PA28INDEX

// LED Blue J4.37
#define BP_RGB_B_PIN 31
#define BP_RGB_B_MASK ((uint32_t) (0x00000001U << BP_RGB_B_PIN))
#define BP_RGB_B_PORT GPIOA
#define BP_RGB_B_INDEX PA31INDEX

// Buzzer J4.340
#define BP_BUZZER_PIN 4
#define BP_BUZZER_MASK ((uint32_t) (0x00000001U << BP_BUZZER_PIN))
#define BP_BUZZER_PORT GPIOB
#define BP_BUZZER_INDEX PB4INDEX

void InitializeBoosterpack(uint32_t ClockFrequency);

/**
 * @brief Read S1 from the Boosterpack.
 * On J22 jumper on the Launchpad, shunt pins 2 and 3 (see LP-MSPM0G3507 schematic)
 * to connect pin PA11 to the BoosterPack
 * @param none
 * @return 1 if S1 is pressed, 0 otherwise
 */
uint32_t BP_ReadS1();

/**
 * @brief Read S2 from the Boosterpack.
 * @param none
 * @return 1 if S2 is pressed, 0 otherwise
 */
uint32_t BP_ReadS2();

/**
 * @brief Turn on RGB red LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOnRedLED(void);

/**
 * @brief Turn off RGB red LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOffRedLED(void);
/**
 * @brief Toggle RGB red LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_ToggleRedLED(void);

/**
 * @brief Turn on RGB green LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOnGreenLED(void);

/**
 * @brief Turn off RGB green LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOffGreenLED(void);
/**
 * @brief Toggle RGB green LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_ToggleGreenLED(void);

/**
 * @brief Turn on RGB blue LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOnBlueLED(void);

/**
 * @brief Turn off RGB blue LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_TurnOffBlueLED(void);
/**
 * @brief Toggle RGB blue LED (D1 on BP).
 * @param none
 * @return none
 */
void BP_ToggleBlueLED(void);

#endif

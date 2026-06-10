#include <ti/devices/msp/msp.h>
#include "../../LP_MSPM0G3507/MKII.h"
#include "../../LP_MSPM0G3507/tone_pitch.h"
#include "../../LP_MSPM0G3507/timer.h"
#include "../../LP_MSPM0G3507/clock.h"
#include "../../LP_MSPM0G3507/LCD.h"
#include "../../LP_MSPM0G3507/mspm0g350x_int.h"
#include "../../LP_MSPM0G3507/i2c.h"
#include "../../LP_MSPM0G3507/opt3001.h"

#define FALSE 0
#define TRUE 1
#define PWM_PRESCALE 31
#define PWM_PERIOD ((ClockFrequency)/((PWM_PRESCALE+1)*(NOTE_A4))) // Clock_Freq/((prescale)(divisor)(frequency))

typedef enum {
    Stop, Start
} FSM_State;

const FSM_State NextStateTable[2][4] =
{
//(S1,S2)=(0,0)  (0,1)  (1,0) (1,1)   Current State
        { Stop,  Stop, Start, Stop }, // Stop
        { Start, Stop, Start, Stop }  // Start
};

// Define FSM State Struct
typedef struct {
    FSM_State currentState;
    FSM_State nextState;
} FSM;

typedef struct{
    uint32_t frequency;
    uint32_t duration_ms;
    const char *name;
} Note;

// PWM Timer
Timer_ClockConfig PWMTimerClockConfig =
{
    .clockSel = GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE,
    .divideRatio = GPTIMER_CLKDIV_RATIO_DIV_BY_1,
    .prescale = PWM_PRESCALE
};
Timer_TimerConfig PWMTimerConfig =
{
    .period = 0,
    .timerMode = (GPTIMER_CTRCTL_CM_DOWN | GPTIMER_CTRCTL_REPEAT_REPEAT_1)
};
Timer_PWMConfig PWMTimerPWMConfig =
{
    .index = 16,
    .iomuxMode = (IOMUX_PINCM_PC_CONNECTED | IOMUX_MODE4),
    .ccr = 0,
    .ccpd = GPTIMER_CCPD_C0CCP0_OUTPUT,
    .ccctl = GPTIMER_CCCTL_01_COC_COMPARE,
    .octl = GPTIMER_OCTL_01_CCPO_FUNCVAL,
    .ccact = (GPTIMER_CCACT_01_CDACT_CCP_LOW | GPTIMER_CCACT_01_LACT_CCP_HIGH),
    .duty = 50,
};

// I2C configuration
I2C_Config i2c1_config =
{
    .SCL_index = PB2INDEX,
    .SDA_index = PB3INDEX,
    .ClockFrequency = 0
};

uint32_t setPeriod(uint32_t frequency);

uint32_t speed_factor = 2; //adjusting speed to be more accurate

void displayNote(char *name);

void playNote(uint32_t frequency, uint32_t duration_ms, char *name, float volume);

void Group1CallbackFunction(void *CallbackObject);

uint32_t ClockFrequency = 32000000;

Callback_s Group1 = {(void *) 0, Group1CallbackFunction};

int main(void)
{
    Note song[] = {
    {NOTE_G4, 200, "G4"},
    {NOTE_C5, 400, "C5"},
    {NOTE_G4, 300, "G4"},
    {NOTE_A4, 100, "A4"},
    {NOTE_B4, 400, "B4"},
    {NOTE_E4, 200, "E4"},
    {NOTE_E4, 200, "E4"},
    {NOTE_A4, 400, "A4"},
    {NOTE_G4, 300, "G4"},
    {NOTE_F4, 100, "F4"},
    {NOTE_G4, 400, "G4"},
    {NOTE_C4, 200, "C4"},
    {NOTE_C4, 200, "C4"},
    {NOTE_D4, 400, "D4"},
    {NOTE_D4, 200, "D4"},
    {NOTE_E4, 200, "E4"},
    {NOTE_F4, 400, "F4"},
    {NOTE_F4, 200, "F4"},
    {NOTE_G4, 200, "G4"},
    {NOTE_A4, 400, "A4"},
    {NOTE_B4, 200, "B4"},
    {NOTE_C5, 200, "C5"},
    {NOTE_D5, 600, "D5"},
    {NOTE_G4, 200, "G4"},
    {NOTE_E5, 400, "E5"},
    {NOTE_D5, 300, "D5"},
    {NOTE_C5, 100, "C5"},
    {NOTE_D5, 400, "D5"},
    {NOTE_B4, 200, "B4"},
    {NOTE_G4, 200, "G4"},
    {NOTE_C5, 400, "C5"},
    {NOTE_B4, 300, "B4"},
    {NOTE_A4, 100, "A4"},
    {NOTE_B4, 400, "B4"},
    {NOTE_E4, 200, "E4"},
    {NOTE_E4, 200, "E4"},
    {NOTE_A4, 400, "A4"},
    {NOTE_G4, 200, "G4"},
    {NOTE_F4, 200, "F4"},
    {NOTE_G4, 400, "G4"},
    {NOTE_C4, 200, "C4"},
    {NOTE_C4, 200, "C4"},
    {NOTE_C5, 400, "C5"},
    {NOTE_B4, 300, "B4"},
    {NOTE_A4, 100, "A4"},
    {NOTE_G4, 800, "G4"}};

    uint32_t songLength = sizeof(song) / sizeof(song[0]);

    // Initialization section: MKII BoosterPack and variables.
    InitializeBoosterpack(ClockFrequency);
    
    uint32_t priority = 1;
    FSM Song_FSM = {NextStateTable[0][0], NextStateTable[0][0]};
    Group1.CallbackObject = (void *) &Song_FSM; // Update Group 1 callback object pointer.

    __disable_irq();

    BP_S1_PORT->POLARITY15_0 = BP_S1_POL_RISING;    // Falling-edge interrupt
    BP_S1_PORT->CPU_INT.ICLR = BP_S1_MASK;            // Clear interrupt bit
    BP_S1_PORT->CPU_INT.IMASK = BP_S1_MASK;           // Enable the S1 interrupt.

    BP_S2_PORT->POLARITY15_0 |= BP_S2_POL_RISING;    // Rising-edge interrupt
    BP_S2_PORT->CPU_INT.ICLR |= BP_S2_MASK;            // Clear interrupt bit
    BP_S2_PORT->CPU_INT.IMASK |= BP_S2_MASK;           // Enable the S2 interrupt.

    // Enable NVIC interrupt and set interrupt priority.
    __NVIC_EnableIRQ(BP_S1_INTERRUPT);
    __NVIC_SetPriority(BP_S1_INTERRUPT,priority);
    __NVIC_EnableIRQ(BP_S2_INTERRUPT);
    __NVIC_SetPriority(BP_S2_INTERRUPT,priority);

    __enable_irq();

    // Adjust the PWM period and delta values based on the clock divide
    // ratio and prescale for the PWM timer.
    PWMTimerConfig.period = PWM_PERIOD;

    // Configure timer G6 to generate the PWM output.
    InitializeTimerClock(TIMA1,&PWMTimerClockConfig);
    InitializeTimerCompare(TIMA1,&PWMTimerConfig);
    InitializeTimerPWM(TIMA1,&PWMTimerPWMConfig);
    
    LCD_Init();

    i2c1_config.ClockFrequency = ClockFrequency;
    InitializeI2C(I2C1,&i2c1_config);

    InitializeOpt3001Sensor(I2C1);
    
    UpdateDutyCycle(TIMA1,&PWMTimerPWMConfig);

    // Infinite loop
    const uint8_t restTime = 10;
    uint8_t index = 0;
    float volume;

    while (TRUE) {
        Song_FSM.currentState = Song_FSM.nextState;

        if (Song_FSM.currentState == Stop) {
            LCD_FillScreen(LCD_BLACK);

            PWMTimerConfig.period = 0;
            InitializeTimerCompare(TIMA1, &PWMTimerConfig);
            EnableTimer(TIMA1);
            PWMTimerPWMConfig.duty = 0;
            UpdateDutyCycle(TIMA1, &PWMTimerPWMConfig);
            
            continue;
        }

        uint16_t lum = sensorOpt3001Read(I2C1, RESULT_REGISTER_ADDRESS);
        float lux = sensorOpt3001Convert(lum);
        float volume = lux / 11;
        if (volume > 100) volume = 100;

        playNote(song[index].frequency, song[index].duration_ms - restTime, (char *) song[index].name, volume);
        ClockDelay_1ms(restTime);

        index++;

        if (index == songLength) {
            LCD_FillScreen(LCD_BLACK);
            ClockDelay_1ms(1000); // Wait one second to restart the song
            index = 0;
        }
    }

    return 0;
}

uint32_t setPeriod(uint32_t frequency)
{
    if (frequency == 0) {
        return 1;
    }
    return ((ClockFrequency)/((PWM_PRESCALE+1)*(frequency))); // Clock_Freq/((prescale)(divisor)(frequency)))
}

void displayNote(char *name) {
    LCD_FillScreen(LCD_BLACK);
    LCD_SetTextColor(LCD_CYAN);
    if (name[0]) {
        LCD_OutString("\n\n\n\n\n\n");
        LCD_OutString("          ");
        LCD_OutString(name);     
        LCD_OutString("\n\n\n\n\n\n\n\n\n\n");
    }
}

void playNote(uint32_t frequency, uint32_t duration_ms, char *name, float volume)
{
    PWMTimerConfig.period = setPeriod(frequency);
    InitializeTimerCompare(TIMA1, &PWMTimerConfig);
    EnableTimer(TIMA1);
    PWMTimerPWMConfig.duty = (uint32_t) volume;
    UpdateDutyCycle(TIMA1, &PWMTimerPWMConfig);

    displayNote(name);

    ClockDelay_1ms(speed_factor*duration_ms); 

    PWMTimerPWMConfig.duty = 0;
    UpdateDutyCycle(TIMA1, &PWMTimerPWMConfig);
}

void Group1CallbackFunction(void *CallbackObject)
{
    FSM *pwm = (FSM *)CallbackObject;

    uint32_t IIDXValue = GPIOA->CPU_INT.IIDX;

    uint8_t TableRow = (uint8_t) (pwm->currentState == Start);
    uint8_t TableCol = (((uint8_t) (IIDXValue == (BP_S1_PIN+1))) << 1) | ((uint8_t) (IIDXValue == (BP_S2_PIN+1)));

    pwm->nextState = NextStateTable[TableRow][TableCol];
}
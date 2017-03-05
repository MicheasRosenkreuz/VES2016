#include <math.h>
#include <stdint.h>
#include <string.h>
#include "pio.h"
#include "cp15.h"
#include "trace.h"
#include "..\Drivers\cpu\timers.h"
#include "dbguart.h"
#include "CommandUI.h"
#include "adc.h"

typedef unsigned char BOOL;

#define PIN_LED_6   {1 << 4, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_7   {1 << 5, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_8   {1 << 9, AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_9   {1 << 6, AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}

static const Pin pinsLeds[] = {PINS_LEDS};

void LED_Set(unsigned int led, BOOL bSet){
    // Check if LED exists
    if (led >= PIO_LISTSIZE(pinsLeds))
        return;
    // Turn LED on
    if (pinsLeds[led].type == PIO_OUTPUT_0){
        if (bSet) PIO_Set(&pinsLeds[led]);
        else PIO_Clear(&pinsLeds[led]);
    }else{
        if (bSet) PIO_Clear(&pinsLeds[led]);
        else PIO_Set(&pinsLeds[led]);
    }
}

#define PIN_TIOB1 {1 << 7, AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}
static const Pin pinsTC[] = { PIN_TIOB1};

uint16_t pwmPeriod = 1024;
uint32_t tckLastAdc = 0;
unsigned char lightStat = 1;
int freq = 0;
BOOL direction = TRUE;

int main (void)
{
    // DBGU configuration
    // TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    // #define PINS_DBGU  {(1<<14) | (1<<15), AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
    const Pin pinsDbgu[] = {PINS_DBGU};
    PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu));
    DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK);

    printf("\r\n\n---------------------------------------\n\r");
    printf("-- Example 1 -  Template Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s    --\n\r", __DATE__, __TIME__);
    printf("---------------------------------------\n\r");
    static volatile int i=0;

    PIO_Configure (pinsLeds, PIO_LISTSIZE(pinsLeds));
    CP15_Enable_I_Cache();
    TimerPeriodicInit();

    uint32_t dwLastTicks = GetTickCount();

    char buffIn[16];
    int buffCnt = 0;

    static const Pin pinsADC[] = {PINS_ADC};
    PIO_Configure(pinsADC, PIO_LISTSIZE(pinsADC));

    ADC_Initialize( AT91C_BASE_ADC,
                    AT91C_ID_ADC,
                    AT91C_ADC_TRGEN_DIS,
                    0,
                    AT91C_ADC_SLEEP_NORMAL_MODE,
                    AT91C_ADC_LOWRES_10_BIT,
                    BOARD_MCK,
                    ADC_MAX_CK_10BIT,
                    10,
                    1200);


    ADC_EnableChannel(AT91C_BASE_ADC, ADC_CHANNEL_0);
    ADC_EnableChannel(AT91C_BASE_ADC, ADC_CHANNEL_1);


    int adcState = 0;    //0 .. start, 1 .. wait
    uint16_t adc0, adc1;
    int nSamples = 0;

    PMC_EnablePeripheral(AT91C_ID_TC1);
    *AT91C_TC1_CCR = AT91C_TC_CLKDIS ; /* Disable the Clock counter */
//    PIO_Configure (pinsLight, PIO_LISTSIZE(pinsLight));
    PIO_Configure(pinsTC, PIO_LISTSIZE(pinsTC));

    unsigned tcclks;

    tcclks = AT91C_TC_CLKS_TIMER_DIV4_CLOCK;     //MCK/128 = 99328000/128 = 776000 Hz
    //tcclks = AT91C_TC_CLKS_TIMER_DIV5_CLOCK;       // SLCK cca 32768 Hz

    TC_Configure(AT91C_BASE_TC1, tcclks | AT91C_TC_WAVE
                 | AT91C_TC_BCPB_SET
                 | AT91C_TC_BCPC_CLEAR
                 | AT91C_TC_EEVT_XC0
                 | AT91C_TC_CPCTRG);
    AT91C_BASE_TC1->TC_RB = pwmPeriod-1;
    AT91C_BASE_TC1->TC_RC = pwmPeriod;

    *AT91C_TC1_CCR = AT91C_TC_CLKEN ; /* Enable the Clock counter */
    *AT91C_TC1_CCR = AT91C_TC_SWTRG ; /* Trig the timer */


    while (1)
    {
        switch (adcState)
        {
            case 0:
                if (ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC0)) break;
                if (ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC1)) break;
                ADC_StartConversion(AT91C_BASE_ADC);
                adcState = 1;
                break;
            case 1:
                if (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC0)) break;
                if (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC1)) break;
                adc0 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_0);
                adc1 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_1);
                nSamples++;
                adcState = 0;
                break;
        }

        freq = (double)(adc0 - 1) * (900.0/1022.0) + 100;
        uint32_t tck = GetTickCount();
        if (tck - tckLastAdc >= 1000){
            tckLastAdc = tck;
//            printf("ADC CH0:%04x / %d  freq: %d nSamples:%d \n", adc0, adc0, freq, nSamples);
            printf("freq: %d ms \n", freq);
            nSamples = 0;
        }


        if (DBGU_IsRxReady())
        {
            char c = DBGU_GetChar();
            DBGU_PutChar(c);
            if (c == 0x08)
            {
                if (buffCnt != 0) buffCnt--;
            }
            else
            {
                if (buffCnt < sizeof(buffIn))
                {
                    buffIn[buffCnt++] = c;
                }
                else
                {
                    buffIn[buffCnt-1] = c;
                }
            }
        }

        if (buffCnt != 0 &&
                (buffIn[buffCnt-1] == 0x0d || buffIn[buffCnt-1] == 0x0a)
           )
        {
            buffIn[buffCnt-1] = 0;
            if (buffIn[0] != 0)
            {
                UIProcessCmd(buffIn);
            }
            memset(buffIn, 0, sizeof(buffIn));
            buffCnt = 0;
        }

        uint32_t ticks = GetTickCount();
        int idx;
        for (idx=0; idx < 10; idx++)
        {
            if (!led_Status[idx].on)
            {
                if (led_Status[idx].state)
                {
                    LED_Set(idx, 0);
                    led_Status[idx].state = 0;
                }
            }
            else
            {
                if (led_Status[idx].period == 0)
                {
                    if (!led_Status[idx].state)
                    {
                        LED_Set(idx, 1);
                        led_Status[idx].state = 1;
                    }
                }
                else if (ticks - led_Status[idx].ticks >= led_Status[idx].period)
                {
                    led_Status[idx].state = led_Status[idx].state ? 0 : 1;
                    LED_Set(idx, led_Status[idx].state);
                    led_Status[idx].ticks = ticks;
                }
            }
        }


        if (GetTickCount() - dwLastTicks > freq){
            dwLastTicks = GetTickCount();
            LED_Set(0, (lightStat == 1));
            LED_Set(1, (lightStat == 2));
            LED_Set(2, (lightStat == 4));
            LED_Set(3, (lightStat == 8));
            LED_Set(4, (lightStat == 16));
            LED_Set(5, (lightStat == 32));
            if (lightStat == 32 || lightStat == 1)
                direction = !direction;
            if (direction)
                lightStat = lightStat >> 1;
            else
                lightStat = lightStat << 1;
        }
    }

    return 0;
}

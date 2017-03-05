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

#define PINS_LEDSEXT   PIN_LED_6, PIN_LED_7, PIN_LED_8, PIN_LED_9


static const Pin pinsLeds[] = {PINS_LEDS, PINS_LEDSEXT};

void LED_Set(unsigned int led, BOOL bSet)
{
    // Check if LED exists
    if (led >= PIO_LISTSIZE(pinsLeds))
    {

        return;
    }

    // Turn LED on
    if (pinsLeds[led].type == PIO_OUTPUT_0)
    {
        if (bSet) PIO_Set(&pinsLeds[led]);
        else PIO_Clear(&pinsLeds[led]);
    }
    else
    {
        if (bSet) PIO_Clear(&pinsLeds[led]);
        else PIO_Set(&pinsLeds[led]);
    }

}

uint32_t tckLastAdc = 0;

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
    static volatile int j=0;

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

        uint32_t tck = GetTickCount();
        if (tck - tckLastAdc >= 500)
        {
            float Upot = (float)adc0 * 3.3F/1023.0F;
            float Urt = (float)adc1 * 3.3F/1023.0F;
            float Rt = (10000.0F * Urt)/(3.3F - Urt);
            //NTC 47kOhmu pri 25stC, B = 4350
            float T = 4350.0F/(logf(Rt/(47000.0F*expf(-4350.0F/(25.0F+273.15F)))));
            float Tc = T - 273.15F;
            tckLastAdc = tck;
            printf("ADC CH0:%04x CH1:%04x nSamples:%d ", adc0, adc1, nSamples);
            printf ("Upot: %d.%02d", (int32_t)Upot, (int32_t)(Upot*100.0F)%100);
            printf ("Urt: %d.%02d", (int32_t)Urt, (int32_t)(Urt*100.0F)%100);
            printf ("Rt: %5d.%02d", (int32_t)Rt, (int32_t)(Rt*100.0F)%100);
            printf ("T: %5d.%02d", (int32_t)Tc, (int32_t)(Tc*100.0F)%100);
            printf ("\n\r");
            nSamples = 0;
        }

        //continue;

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


//        if (GetTickCount() - dwLastTicks > 250)
//        {
//            dwLastTicks = GetTickCount();
//
//
//            LED_Set(0, (j & (1 << 0)) != 0);
//            LED_Set(1, (j & (1 << 1)) != 0);
//            LED_Set(2, (j & (1 << 2)) != 0);
//            LED_Set(3, (j & (1 << 3)) != 0);
//            LED_Set(4, (j & (1 << 4)) != 0);
//            LED_Set(5, (j & (1 << 5)) != 0);
//
//            LED_Set(6, (j & (1 << 2)) != 0);
//            LED_Set(7, (j & (1 << 3)) != 0);
//            LED_Set(8, (j & (1 << 4)) != 0);
//            LED_Set(9, (j & (1 << 5)) != 0);
//            j++;
//        }
    }

    return 0;
}

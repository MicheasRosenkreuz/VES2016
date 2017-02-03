#include <math.h>
#include <string.h>
#include "pio.h"
#include "cp15.h"
#include "trace.h"
#include "adc.h"
#include "..\Drivers\cpu\timers.h"


typedef unsigned char BOOL;


static const Pin pinsLeds[] = {PINS_LEDS};

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


void VypisPametiW(unsigned long nAddr, int nCnt)
{
    unsigned long *pMemory = (unsigned long *)nAddr;
    int i;
    for (i=0; i < nCnt; i++)
    {
        if (i % 8 == 0)
        {
            if (i != 0) printf ("\n\r");
            printf ("%08X  ", (unsigned long)pMemory);
        }
        printf("%08X ", *pMemory);
        pMemory ++;
    }
}


char buf[16] = {0};
int buffCnt = 0;
char buffIn[64] = {0};
static int ledState = 0;


double computeT(uint16_t ad, double *pUrt, double *pRt)
{
    const double r26 = 10000.0;
    const R0 = 47000.0;
    const double B = 4350.0;
    const double T0K = 273.15;
    const double e2 = exp (-B / (T0K + 20.0));
    *pUrt = ad * 3.3 / 1023.0;
    *pRt = (r26 * *pUrt)/(3.3 - *pUrt);
    double T = B / log(*pRt/(R0 * e2));
    T -= T0K;
    return T;
}

static const Pin pinsADC[] = {PINS_ADC};

int main (void)
{
    // DBGU configuration
//    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
// #define PINS_DBGU  {(1<<14) | (1<<15), AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
    const Pin pinsDbgu[] = {PINS_DBGU};
    PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu));
    DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK);

    printf("\r\n\n---------------------------------------\n\r");
    printf("-- Example 1 -  Template Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s    --\n\r", __DATE__, __TIME__);
    printf("---------------------------------------\n\r");

///// LED #0 pin definition.
//#define PIN_LED_0   {1 << 24, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
///// LED #1 pin definition.
//#define PIN_LED_1   {1 << 25, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
///// LED #2 pin definition.
//#define PIN_LED_2   {1 << 26, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
///// LED #3 pin definition.
//#define PIN_LED_3   {1 << 27, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
///// LED #4 pin definition.
//#define PIN_LED_4   {1 << 6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
///// LED #5 pin definition.
//#define PIN_LED_5   {1 << 9, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
///// List of all LED definitions.
//#define PINS_LEDS   PIN_LED_4, PIN_LED_5, PIN_LED_0, PIN_LED_1, PIN_LED_2, PIN_LED_3
//static const Pin pinsLeds[] = {PINS_LEDS};

    PIO_Configure (pinsLeds, PIO_LISTSIZE(pinsLeds));

    CP15_Enable_I_Cache();

    TimerPeriodicInit();

    uint32_t dwLastTicks = GetTickCount();

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
    while (1)
    {

        if (DBGU_IsRxReady())
        {
            char c = DBGU_GetChar();
            if (c == 0x08)
            {
                if (buffCnt != 0)
                {
                    buffCnt--;
                    DBGU_PutChar(c);
                }
            }
            else
            {
                if (buffCnt < sizeof(buffIn))
                {
                    buffIn[buffCnt++] = c;
                    DBGU_PutChar(c);
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
                printf ("\nPrikaz: %s\n", buffIn);
            }
            memset(buffIn, 0, sizeof(buffIn));
            buffCnt = 0;
        }


        if (GetTickCount() - dwLastTicks > 250)
        {
            dwLastTicks = GetTickCount();

            LED_Set(0, (ledState & (1 << 0)) != 0);
            LED_Set(1, (ledState & (1 << 1)) != 0);
            LED_Set(2, (ledState & (1 << 2)) != 0);
            LED_Set(3, (ledState & (1 << 3)) != 0);
            LED_Set(4, (ledState & (1 << 4)) != 0);
            LED_Set(5, (ledState & (1 << 5)) != 0);

            ledState++;


            ADC_StartConversion(AT91C_BASE_ADC);
            while (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC0));
            while (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC1));
            int adc0 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_0);
            int adc1 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_1);
            double Urt, Rt;
            double T = computeT(adc1, &Urt, &Rt);
            printf ("ADC0=%5d, ADC1=%5d, T=%8d, Urt=%8d, Rt=%8d\n", adc0, adc1, (int)(T*100), (int)(Urt*100), (int)(Rt*100));
        }
    }

    return 0;
}

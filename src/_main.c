#include <math.h>
#include <string.h>
#include "pio.h"
#include "cp15.h"
#include "trace.h"
#include "..\Drivers\cpu\timers.h"


typedef unsigned char BOOL;
//typedef unsigned short WORD;


static const Pin pinsLeds[] = {PINS_LEDS};
static const Pin pinsLCD[] = {PINS_LCD};

void LED_Set(unsigned int led, BOOL bSet)
{
    // Check if LED exists
    if (led >= PIO_LISTSIZE(pinsLeds)) {

        return;
    }

    // Turn LED on
    if (pinsLeds[led].type == PIO_OUTPUT_0) {
        if (bSet) PIO_Set(&pinsLeds[led]);
        else PIO_Clear(&pinsLeds[led]);
    }
    else {
        if (bSet) PIO_Clear(&pinsLeds[led]);
        else PIO_Set(&pinsLeds[led]);
    }

}

void ConfigureLCDInterface(void)
{
    AT91C_BASE_SMC->SMC_SETUP4 = 0x01030103;
    AT91C_BASE_SMC->SMC_PULSE4 = 0x0a070a07;
    AT91C_BASE_SMC->SMC_CYCLE4 = 0x000c000c;
//    AT91C_BASE_SMC->SMC_SETUP4 = 0x010a010a;
//    AT91C_BASE_SMC->SMC_PULSE4 = 0x1f191F19;
//    AT91C_BASE_SMC->SMC_CYCLE4 = 0x002a002a;

//    AT91C_BASE_SMC->SMC_SETUP4 = 0x010a101f;
//    AT91C_BASE_SMC->SMC_PULSE4 = 0x1f192F19;
//    AT91C_BASE_SMC->SMC_CYCLE4 = 0x002a004f;

    AT91C_BASE_SMC->SMC_CTRL4  = (AT91C_SMC_READMODE
                                  | AT91C_SMC_WRITEMODE
                                  | AT91C_SMC_NWAITM_NWAIT_DISABLE
                                  | ((0x1 << 16) & AT91C_SMC_TDF));

 //   else if (busWidth == 16) {
    AT91C_BASE_SMC->SMC_CTRL4 |= AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS;
    PIO_SetPeripheralA(AT91C_BASE_PIOC, 1 << 8, 1);
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


volatile WORD w;
char buf[16] = {0};

int main (void)
{
    // DBGU configuration
//    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
// #define PINS_DBGU  {(1<<14) | (1<<15), AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
    const Pin pinsDbgu[] = {PINS_DBGU}; \
    PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu)); \
    DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK); \

    printf("\r\n\n---------------------------------------\n\r");
    printf("-- Example 1 -  Template Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s    --\n\r", __DATE__, __TIME__);
    printf("---------------------------------------\n\r");
    static volatile int i=0;
    static volatile int j=0;

    PIO_Configure (pinsLeds, PIO_LISTSIZE(pinsLeds));
    PIO_Configure (pinsLCD, PIO_LISTSIZE(pinsLCD));
    ConfigureLCDInterface();
    CP15_Enable_I_Cache();

    TimerPeriodicInit();

//    lcdInit();
//    lcdTest();

//    Init_LCD();
//    Lcd_On();
//    Lcd_test();
//    Lcd_Fill(RGB24ToRGB16(COLOR_BLUE));
//    LCD_setPixel (10,10,RGB24ToRGB16(COLOR_BLACK));
//    LCD_setPixel (20,20,RGB24ToRGB16(COLOR_BLACK));
    uint32_t dwLastTicks = GetTickCount();

    //VypisPametiW(0x20000000, 64);

    char buffIn[16];
    int buffCnt = 0;

    while (1)
    {
        double f1 = 3.14/2;

//        _I_Disable();
//        *BASE_LCD = 0xaa55;
//        w = *BASE_LCD;
//        _I_Enable();
#ifdef _DEBUG
//        if (i++ %200000 == 0)
#else
//        if (i++ %1000000 == 0)
#endif

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
                printf ("\nPrikaz: %s\n", buffIn);
            }
            memset(buffIn, 0, sizeof(buffIn));
            buffCnt = 0;
        }

        if (GetTickCount() - dwLastTicks > 250)
        {
            dwLastTicks = GetTickCount();

            LED_Set(0, (j & (1 << 0)) != 0);
            LED_Set(1, (j & (1 << 1)) != 0);
            LED_Set(2, (j & (1 << 2)) != 0);
            LED_Set(3, (j & (1 << 3)) != 0);
            LED_Set(4, (j & (1 << 4)) != 0);
            LED_Set(5, (j & (1 << 5)) != 0);

//            printf ("01");
//            f1 = sin(f1);
//            if (j & (1 << 6)) PIO_Set(&pinsLCD[PINIDX_LCD_BL]);
//            else PIO_Clear(&pinsLCD[PINIDX_LCD_BL]);


            j++;
        }
    }

	return 0;
}

#include <math.h>
#include <string.h>
#include "pio.h"
#include "cp15.h"
#include "trace.h"
#include "..\Drivers\cpu\timers.h"


typedef unsigned char BOOL;

static const Pin pinsLeds[] = {PINS_LEDS};

void LED_Set(unsigned int led, BOOL bSet){
    // Check if LED exists
    if (led >= PIO_LISTSIZE(pinsLeds)){

        return;
    }

    // Turn LED on
    if (pinsLeds[led].type == PIO_OUTPUT_0){
        if (bSet) PIO_Set(&pinsLeds[led]);
        else PIO_Clear(&pinsLeds[led]);
    }
    else{
        if (bSet) PIO_Clear(&pinsLeds[led]);
        else PIO_Set(&pinsLeds[led]);
    }

}

int main (void)
{
    // DBGU configuration
    // TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    // #define PINS_DBGU  {(1<<14) | (1<<15), AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
    const Pin pinsDbgu[] = {PINS_DBGU};\
    PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu));\
    DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK);\

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
    char *delimiter = " ";
    char *token;
    int led_id;
    int led_period;
    uint isSet;
    int leds[] = {0,0,0,0,0,0};
    int led_tics[] = {0,0,0,0,0,0};
    int led_state[] = {0,0,0,0,0,0};


    while (1){
        if (DBGU_IsRxReady()){
            char c = DBGU_GetChar();
            DBGU_PutChar(c);
            if (c == 0x08){
                if (buffCnt != 0) buffCnt--;
            }
            else{
                if (buffCnt < sizeof(buffIn)){
                    buffIn[buffCnt++] = c;
                }
                else{
                    buffIn[buffCnt-1] = c;
                }
            }
        }

        if (buffCnt != 0 &&
                (buffIn[buffCnt-1] == 0x0d || buffIn[buffCnt-1] == 0x0a)
           ){
            buffIn[buffCnt-1] = 0;
            if (buffIn[0] != 0){
/////////////////////////////////////////////////////////////////////////////
                printf ("\nPrikaz: %s\n", buffIn);

                token = strtok(buffIn, delimiter);
                if(strcmp(token, "SET") == 0){
                    isSet = 1;
                }if(strcmp(token, "CLR") == 0){
                    isSet = 0;
                }
                token = strtok(NULL, delimiter);
                led_id = atoi(token);
                leds[led_id] = isSet;
                token = strtok(NULL, delimiter);
                led_period = atoi(token);
                if(isSet){
                    led_tics[led_id] = led_period;
                }/*
                for(i=0 ; i<6; i++){
                    printf("%d", leds[i]);
                }*/
            }
            memset(buffIn, 0, sizeof(buffIn));
            buffCnt = 0;
        }
        int i;
        for(i = 0; i < 6 ; i++){
            if (GetTickCount() - dwLastTicks > led_tics[i]){
                dwLastTicks = GetTickCount();
                led_state[i] = (led_state[i]+1)%2;
                LED_Set(i, leds[i] && led_state[i]);


                j++;
            }
        }
    }

    return 0;
}
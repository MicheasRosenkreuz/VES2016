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
    char *token;  // temporary string for str split
    char *delimiter = " ";
    char *cmd;  // SET/CLR
    int led_id;  // id of LED
    int led_period; // period of blinking
    uint isSet = 0;  // is SET command obtained
    int leds[] = {1,0,0,0,0,0};  // Enabled LEDs
    int led_tics[] = {100,0,0,0,0,0};  // Period in ms for each LED
    int led_state[] = {0,0,0,0,0,0};  // State of blink for each LED


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
                printf ("\n>>>%s\n", buffIn);
                // parse cmd
                token = strtok(buffIn, delimiter);
                cmd = token;
                token = strtok(NULL, delimiter);
                led_id = atoi(token);
                token = strtok(NULL, delimiter);
                led_period = atoi(token);

                if(strcmp(cmd, "SET") == 0){
                    isSet = 1;
                }if(strcmp(cmd, "CLR") == 0){
                    isSet = 0;
                }
                leds[led_id] = isSet;
                if(isSet){
                    led_tics[led_id] = led_period;
                }
                // DBG Info
                printf("\nDBG> \ncmd: \'%s\' \'%d\' \'%d\'\nisSet: %d\n", cmd, led_id, led_period, isSet);
                printf("Enabled LEDs [leds]: \n");
                for(i=0 ; i<6; i++)
                    printf("%d", leds[i]);
                printf("\nPeriod in ms [led_tics]\n");
                for(i=0 ; i<6; i++)
                    printf("%d,", led_tics[i]);
                printf("\nState of blink [led_state]\n");
                for(i=0 ; i<6; i++)
                    printf("%d", led_state[i]);

            }
            memset(buffIn, 0, sizeof(buffIn));
            buffCnt = 0;
        }
        int i;
        if (GetTickCount() - dwLastTicks > 250)
        {
            dwLastTicks = GetTickCount();
            led_state[0] = !led_state[0];
            led_state[1] = !led_state[1];
            LED_Set(0, leds[0] & led_state[0]);
            LED_Set(1, leds[1] & led_state[1]);
            LED_Set(2, (j & (1 << 2)) != 0);
            LED_Set(3, (j & (1 << 3)) != 0);
            LED_Set(4, (j & (1 << 4)) != 0);
            LED_Set(5, (j & (1 << 5)) != 0);
            j++;
        /*for(i = 0; i < 6 ; i++){
            if (GetTickCount() - dwLastTicks > led_tics[i]){
                dwLastTicks = GetTickCount();
                led_state[i] = (led_state[i]+1)%2;
                LED_Set(i, (leds[i] != 0) && (led_state[i] != 0));
            }*/
        }
    }

    return 0;
}

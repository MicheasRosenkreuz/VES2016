///////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "pio.h"
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include "DbgUart.h"



#define TXBUFLENGTH     1024
#define RXBUFLENGTH     16
#define RXCANBUFLENGTH  32
#define CMDBUFLENGTH    64

static volatile char bufTx[TXBUFLENGTH];
static volatile char bufRx[RXBUFLENGTH];
static volatile char bufRxCan[RXCANBUFLENGTH];
static volatile char bufCmd[CMDBUFLENGTH];
static volatile char prevCmd[CMDBUFLENGTH];

static volatile uint16_t posWTx;
static volatile uint16_t posRTx;
static volatile uint16_t posRTxCan;
static volatile uint16_t posRRx;
static volatile uint16_t posCmd;
static volatile uint8_t  complCmd;
static volatile uint16_t posWRxCan;
static volatile uint16_t posRRxCan;


void DbgUartInit()
{
    posWTx = 0;
    posRTx = 0;
    posRRx = 0;
    posCmd = 0;
    bufCmd[0] = 0;
    prevCmd[0] = 0;
    posWRxCan = 0;
    posRRxCan = 0;
}




void DbgSendStr(const char *str)
{
    int idx;
    for(idx=0; str[idx] != 0; idx++)
    {
        DBGU_PutChar(str[idx]);
    }
}

void DbgSendStrNl(const char *str)
{
    DbgSendStr(str);
    DbgSendStr(DBGSTRNL);
}

void DbgFormat ( const char * format, ... )
{
  char buffer[200+1];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer,sizeof(buffer)-1, format, args);
  va_end (args);
  DbgSendStr(buffer);
}




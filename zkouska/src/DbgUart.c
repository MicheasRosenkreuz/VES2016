///////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "pio.h"
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include "DbgUart.h"



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




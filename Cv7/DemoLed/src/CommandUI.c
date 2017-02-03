#include <string.h>
#include <stdint.h>
#include "pio.h"
#include "adc.h"
#include "dbguart.h"
#include "CommandUI.h"
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void DoCmdHelp()
{
    DbgSendStrNl("----------- Terminal Commands List -----------");
    DbgSendStrNl("SET            LED Set <1..10>, period <ms>");
    DbgSendStrNl("CLR            LED OFF <1..10>");
}



void DoCmdSet()
{
    char *sp1 = strtok(NULL, " ,");
    char *sp2 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing LED index");
        return;
    }
    int index = atoi(sp1);
    if (index < 1 || index > 10)
    {
        DbgSendStrNl("LED index out of range");
        return;
    }

    uint32_t period = 0;
    if (sp2 != NULL)
    {
        period = atoi(sp2);
    }
    DbgFormat("LED SET %d, period %d"DBGSTRNL, index, period);
}

void DoCmdClr()
{
    char *sp1 = strtok(NULL, " ,");
    char *sp2 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing LED index");
        return;
    }
    int index = atoi(sp1);
    if (index < 1 || index > 10)
    {
        DbgSendStrNl("LED index out of range");
        return;
    }

    DbgFormat("LED CLR %d"DBGSTRNL, index);
}

void DoCmdAd()
{
    ADC_StartConversion(AT91C_BASE_ADC);
    while (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC0));
    while (!ADC_IsStatusSet(AT91C_BASE_ADC, AT91C_ADC_EOC1));
    int adc0 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_0);
    int adc1 = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_CHANNEL_1);

    printf ("ADC0=%5d, ADC1=%5d\n", adc0, adc1);

}

///////////////////////////////////////////////////////////////////////////////
// Command processor                                                         //
///////////////////////////////////////////////////////////////////////////////

void UIProcessCmd(char *cmd)
{
    if (cmd != NULL)
    {
        DbgSendStrNl("");
        char *s1 = strtok((char *)cmd, " ,");
        if (s1 == NULL)
        {
            DbgFormat("Invalid command - %s"DBGSTRNL, cmd);
        }
        else if (stricmp(s1, "H") == 0)
        {
            DoCmdHelp();
        }
        else if (stricmp(s1, "SET") == 0)
        {
            DoCmdSet();
        }
        else if (stricmp(s1, "CLR") == 0)
        {
            DoCmdClr();
        }
        else if (stricmp(s1, "AD") == 0)
        {
            DoCmdAd();
        }
        else
        {
            DbgFormat("Unknown command - %s"DBGSTRNL, cmd);
        }
        DbgSendStr("->");
    }
}

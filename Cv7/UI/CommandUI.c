#include <string.h>
#include <stdint.h>

#include "dbguart.h"
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//#pragma GCC optimize ("O0")

//
//
//void DoCmdDRR()
//{
//    char *sp1 = strtok(NULL, " ,");
//    char *sp2 = strtok(NULL, " ,");
//    if (sp1 == NULL)
//    {
//        DBGUSART_dbgSendStrNl("Missing device address");
//        return;
//    }
//    if (sp2 == NULL)
//    {
//        DBGUSART_dbgSendStrNl("Missing register address");
//        return;
//    }
//    uint32_t d = 0;
//    uint32_t r = 0;
//    uint32_t n = sscanf(sp1, "%lu", &d);
//    if (n != 1)
//    {
//        DBGUSART_dbgSendStrNl("Invalid parameter(s) format");
//        return;
//    }
//    n = sscanf(sp2, "%lu", &r);
//    if (n != 1)
//    {
//        DBGUSART_dbgSendStrNl("Invalid parameter(s) format");
//        return;
//    }
//    if (d == 0 || d >= 32)
//    {
//        DBGUSART_dbgSendStrNl("Invalid device address");
//        return;
//    }
//    if (r > 65535)
//    {
//        DBGUSART_dbgSendStrNl("Invalid register address");
//        return;
//    }
//    uint16_t w[2];
//    int ec = ModbusDrvRequest(MBMST_RQ_RDREGS, (uint8_t)d, (uint16_t)r, 2, w);
//    if (ec != MBMST_ERROR_OK)
//    {
//        DBGUSART_Format("MB DRR Error %d"DBGSTRNL, ec);
//    }
//    else
//    {
//        uint32_t v = w[0];
//        v <<= 16;
//        v |= w[1];
//        DBGUSART_Format("MB DRR %d =  %d(%08x)"DBGSTRNL, r, v, v);
//    }
//}




void DoCmdAD()
{
//    DbgFormat("ADC: %04x %04x %04x %04x %04x %04x %04x %04x" DBGSTRNL, AdcGetValueRaw(0), AdcGetValueRaw(1), AdcGetValueRaw(2), AdcGetValueRaw(3), AdcGetValueRaw(4), AdcGetValueRaw(5), AdcGetValueRaw(6), AdcGetValueRaw(7));
}


void DoCmdOS()
{
    char *sp1 = strtok(NULL, " ,");
    char *sp2 = strtok(NULL, " ,");
    char *sp3 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing channel parameter");
        return;
    }

    if (sp2 == NULL)
    {
        DbgSendStrNl("Missing device specification");
        return;
    }
    if (sp3 == NULL)
    {
        DbgSendStrNl("Missing state");
        return;
    }
    uint32_t v;
}

void DoCmdOV()
{
    char *sp1 = strtok(NULL, " ,");
    char *sp2 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing valve parameter");
        return;
    }
    if (sp2 == NULL)
    {
        DbgSendStrNl("Missing state");
        return;
    }
    uint32_t v;
}



void DoCmdHelp()
{
    DbgSendStrNl("----------- Terminal Commands List -----------");
    DbgSendStrNl("TI             Tasks Info");
    DbgSendStrNl("AD             External ADC Raw Values");
    DbgSendStrNl("P              Display Pressure");
    DbgSendStrNl("OS             Set Out Sands channel<1..4> device<M1,M2,H,B> state<S,R>");
    DbgSendStrNl("OV             Set Out Valve valve<1..5> state<S,R>");
//    DbgFlush();
    DbgSendStrNl("AOFF           Auto Mode OFF");
    DbgSendStrNl("AON            Auto Mode ON");
    DbgSendStrNl("HM             Set Hydr. Mot PWM mot<1,2> pwm<0..100>");
    DbgSendStrNl("VA             Set Valve PWM pwm<0..80>");
    DbgSendStrNl("----------------------------------------------");
//    DbgFlush();
}


void DoCmdTI()
{
    DbgSendStrNl("Name   Time[ns] Last       Max       Min    Remain   Rem.Min");
    DbgSendStrNl("------------------------------------------------------------");
    int idx;
    DbgSendStrNl("------------------------------------------------------------");
}

void DoCmdHM()
{
    char *sp1 = strtok(NULL, " ,");
    char *sp2 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing motor parameter");
        return;
    }
    if (sp2 == NULL)
    {
        DbgSendStrNl("Missing pwm value");
        return;
    }
}

void DoCmdVA()
{
    char *sp1 = strtok(NULL, " ,");
    if (sp1 == NULL)
    {
        DbgSendStrNl("Missing pwm value");
        return;
    }
    uint32_t d = 0;
//    uint32_t n = sscanf(sp1, "%lu", &d);
//    if (n != 1)
//    {
//        DbgSendStrNl("Invalid parameter format");
//        return;
//    }
}


extern uint8_t autoOff;
extern uint8_t motOn;

void DoCmdAOFF()
{
}

void DoCmdAON()
{
}


void DoCmdP()
{
//    DbgFormat("P1: %5.1f; P1: %5.1f"DBGSTRNL, AdcGetPressure(0), AdcGetPressure(1));

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
        else if (stricmp(s1, "TI") == 0)
        {
            DoCmdTI();
        }
        else if (stricmp(s1, "AD") == 0)
        {
            DoCmdAD();
        }
        else if (stricmp(s1, "OS") == 0)
        {
            DoCmdOS();
        }
        else if (stricmp(s1, "OV") == 0)
        {
            DoCmdOV();
        }
        else if (stricmp(s1, "HM") == 0)
        {
            DoCmdHM();
        }
        else if (stricmp(s1, "VA") == 0)
        {
            DoCmdVA();
        }
        else if (stricmp(s1, "AOFF") == 0)
        {
            DoCmdAOFF();
        }
        else if (stricmp(s1, "AON") == 0)
        {
            DoCmdAON();
        }
        else if (stricmp(s1, "P") == 0)
        {
            DoCmdP();
        }
        else
        {
            DbgFormat("Unknown command - %s"DBGSTRNL, cmd);
        }
        DbgSendStr("->");
    }
}

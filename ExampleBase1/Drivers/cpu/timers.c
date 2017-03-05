
#include "..\..\h\board.h"
#include "pit.h"
#include "aic.h"
#include "timers.h"

/// PIT period value in µseconds.
#define PIT_PERIOD          1000

//------------------------------------------------------------------------------
// Global interrupt enable/disable functions
//------------------------------------------------------------------------------
void _I_Disable(void) {
         asm(
        "mrs r0, cpsr\n\t"
        "orr r0,r0,#0x80\n\t"
        "msr cpsr_c,r0\n\t"
        "mov r0, #1"
        );
}

void _I_Enable(void) {
         asm(
        "mrs r0, cpsr\n\t"
        "bic r0,r0,#0x80\n\t"
        "msr cpsr_c,r0\n\t"
        "mov r0, #1"
        );
}



//------------------------------------------------------------------------------
/// Global timestamp in milliseconds since start of application.
//------------------------------------------------------------------------------
static volatile uint32_t timestamp = 0;


//------------------------------------------------------------------------------
/// Handler for PIT interrupt. Increments the timestamp counter.
//------------------------------------------------------------------------------
void ISR_Pit(void)
{
    unsigned int status;

    // Read the PIT status register
    status = PIT_GetStatus() & AT91C_PITC_PITS;
    if (status != 0) {

        // Read the PIVR to acknowledge interrupt and get number of ticks
        timestamp += (PIT_GetPIVR() >> 20);
    }
}

//------------------------------------------------------------------------------
/// Configure the periodic interval timer to generate an interrupt every
/// millisecond.
//------------------------------------------------------------------------------
void TimerPeriodicInit(void)
{
    // Initialize the PIT to the desired frequency
    PIT_Init(PIT_PERIOD, BOARD_MCK / 1000000);

    // Configure interrupt on PIT
    AIC_DisableIT(AT91C_ID_SYS);
    AIC_ConfigureIT(AT91C_ID_SYS, AT91C_AIC_PRIOR_LOWEST, ISR_Pit);
    AIC_EnableIT(AT91C_ID_SYS);
    PIT_EnableIT();

    // Enable the pit
    PIT_Enable();
}

//------------------------------------------------------------------------------
/// Get ticks count from system started in millisecond.
//------------------------------------------------------------------------------
uint32_t GetTickCount(void)
{
    _I_Disable();
    uint32_t ticks = timestamp;
    _I_Enable();
    return ticks;
}

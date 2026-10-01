#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_cputimer.h"

//
// CPU Timer instance and interrupt in PIE used for the RTOS tick.
// Ensure that the INT corresponds to the configured CPUTIMER.
//
#define PORT_TICK_TIMER_BASE    CPUTIMER2_BASE
#define PORT_TICK_TIMER_INT     INT_TIMER2

//
// SW interrupt in PIE to use as the interrupt for portYIELD().
// It is recommended that this be the lowest priority interrupt line in the PIPE.
// SW should not use this interrupt line for anything else.
//
#define PORT_INT_YIELD          INT_FREERTOS

//-------------------------------------------------------------------------------------------------
// Derived Macros. DO NOT EDIT!!
//-------------------------------------------------------------------------------------------------
#define PORT_TICK_TIMER_O_TCR   ( PORT_TICK_TIMER_BASE + CPUTIMER_O_TCR )
#define PORT_PIE_ACK_YIELD      ( 1U << ((( PORT_INT_YIELD & 0xFF00UL ) >> 8U ) - 1U ))
#define PORT_PIE_O_FLAG         ( 0x0CE1U + ( 2U * (( PORT_INT_YIELD & 0xFF00UL ) >> 8U )))
#define PORT_PIE_FLAG_YIELD     ( 1U << (( PORT_INT_YIELD & 0x00FFUL ) - 1U ))

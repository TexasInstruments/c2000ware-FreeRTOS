#include "inc/hw_ints.h"

//-------------------------------------------------------------------------------------------------
// FreeRTOS Port Macros
//-------------------------------------------------------------------------------------------------
#define PORT_INT_YIELD          INT_FREERTOS       
#define PORT_PIE_ACK_YIELD      ( 1U << ((( PORT_INT_YIELD & 0xFF00UL ) >> 8U ) - 1U ))
#define PORT_PIE_O_FLAG         ( 0x0CE1U + ( 2U * (( PORT_INT_YIELD & 0xFF00UL ) >> 8U )))
#define PORT_PIE_FLAG_YIELD     ( 1U << (( PORT_INT_YIELD & 0x00FFUL ) - 1U ))

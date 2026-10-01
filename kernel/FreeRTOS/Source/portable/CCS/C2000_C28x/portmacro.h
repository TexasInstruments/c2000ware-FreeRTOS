/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */
//-------------------------------------------------------------------------------------------------

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// Port specific definitions.
//
// The settings in this file configure FreeRTOS correctly for the
// given hardware and compiler.
//
// These settings should not be altered.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Hardware includes
//-------------------------------------------------------------------------------------------------
#include "portdefines.h"
#include "cputimer.h"

//-------------------------------------------------------------------------------------------------
// Type definitions.
//-------------------------------------------------------------------------------------------------
#define portCHAR        uint16_t
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        uint32_t
#define portSHORT       uint16_t
#define portBASE_TYPE   uint16_t
#define portSTACK_TYPE  uint16_t

typedef portSTACK_TYPE StackType_t;
typedef int16_t        BaseType_t;
typedef uint16_t       UBaseType_t;

#if ( configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_16_BITS )
    typedef uint16_t        TickType_t;
    #define portMAX_DELAY   ( TickType_t ) 0xffff
#elif ( configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_32_BITS )
    typedef uint32_t        TickType_t;
    #define portMAX_DELAY   ( TickType_t ) 0xffffffffUL
#else
    #error configTICK_TYPE_WIDTH_IN_BITS set to unsupported tick type width.
#endif

//-------------------------------------------------------------------------------------------------
// Interrupt control macros.
//-------------------------------------------------------------------------------------------------
#define portDISABLE_INTERRUPTS()  __asm(" setc INTM")
#define portENABLE_INTERRUPTS()   __asm(" clrc INTM")

//-------------------------------------------------------------------------------------------------
// Critical section control macros.
//-------------------------------------------------------------------------------------------------
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );

#define portENTER_CRITICAL()  vPortEnterCritical()
#define portEXIT_CRITICAL()   vPortExitCritical()

//-------------------------------------------------------------------------------------------------
// Task utilities.
//-------------------------------------------------------------------------------------------------
#define portYIELD()             { HWREGH(PORT_PIE_O_FLAG) |= PORT_PIE_FLAG_YIELD; asm(" RPT #9 || NOP"); }
#define portYIELD_FROM_ISR( x ) { if (x != pdFALSE) portYIELD() }

extern void portTICK_ISR( void );
extern void portYIELD_ISR( void );
extern void portRESTORE_FIRST_CONTEXT( void );
extern void vTaskSwitchContext( void );

//-------------------------------------------------------------------------------------------------
// Hardware specifics.
//-------------------------------------------------------------------------------------------------
#define portBYTE_ALIGNMENT      2
#define portSTACK_GROWTH        ( 1 )
#define portTICK_PERIOD_MS      ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portNOP()               __asm(" NOP")

//-------------------------------------------------------------------------------------------------
// Architecture specific optimisations.
//-------------------------------------------------------------------------------------------------
#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

    extern uint16_t portGET_HIGHEST_PRIORITY (uint32_t readyPrioritiesBitMap);

    /* Check the configuration. */
    #if( configMAX_PRIORITIES > 31 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 31.
    #endif

    /* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )      uxReadyPriorities |=  ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )       uxReadyPriorities &= ~( 1UL << ( uxPriority ) )

    /*-----------------------------------------------------------*/

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = portGET_HIGHEST_PRIORITY(uxReadyPriorities)

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

//-------------------------------------------------------------------------------------------------
// Task function macros as described on the FreeRTOS.org WEB site.
//-------------------------------------------------------------------------------------------------
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* PORTMACRO_H */

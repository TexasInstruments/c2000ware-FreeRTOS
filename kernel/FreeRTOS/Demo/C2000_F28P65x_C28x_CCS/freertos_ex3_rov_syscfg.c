//#############################################################################
//
// FILE:   freertos_ex3_rov_syscfg.c
//
// TITLE:  FreeRTOS ROV demonstration (SYSCONFIG)
//
//! This example is used to demonstrate the ROV functionality part of the 
//! CCS Theia IDE. Sysconfig is used to set up the FreeRTOS application, 
//! and enable ROV configurations. To use ROV, start debug session and  
//! open View -> Runtime (RTOS) Objects. Various parameters related to stack  
//! and heap usage, task execution state, queue status etc. can be inspected 
//! using the ROV view.
//!  
//! NOTE: ROV functionality is compatible only with CCS Theia
//!
//! External Connections:
//! None
//!
//! Watch Variables:
//!  - timerCount - Number of times the RTOS timer has completed countdown
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.co/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

#include "driverlib.h"
#include "device.h"     // Device Headerfile and Examples Include File
#include "FreeRTOS.h"
#include "board.h"
#include "c2000_freertos.h"

//
// Globals
//

#define RED         0xDEADBEAF
#define BLUE        0xBAADF00D
uint32_t timerCount = 0;

//
// Function Prototypes
//
void vApplicationMallocFailedHook( void );

//-------------------------------------------------------------------------------------------------

//
// myTimer1 Callback Function
//
void myTimerCallback( TimerHandle_t xTimer )
{
    timerCount++;
    xSemaphoreGive( binarySem1Handle );
}

//-------------------------------------------------------------------------------------------------
static void blueLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_writePin(myLED1_GPIO, counter & 1);
}

//-------------------------------------------------------------------------------------------------
static void redLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_writePin(myLED2_GPIO, counter & 1);
}

//-------------------------------------------------------------------------------------------------
static void ledToggle(uint32_t led)
{
    if(RED == led)
    {
        redLedToggle();
    }
    else
    if(BLUE == led)
    {
        blueLedToggle();
    } 
}

//-------------------------------------------------------------------------------------------------
void LED_TaskRed(void * pvParameters)
{
    for(;;)
    {
        if(xSemaphoreTake( binarySem1Handle, portMAX_DELAY ) == pdTRUE)
        {
            ledToggle((uint32_t)pvParameters);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void LED_TaskBlue(void * pvParameters)
{
    for(;;)
    {
        ledToggle((uint32_t)pvParameters);
        if( xSemaphoreTake( mutex1Handle, ( TickType_t ) 10 ) == pdTRUE )
        {
            xSemaphoreGive( mutex1Handle );
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

//-------------------------------------------------------------------------------------------------
void main(void)
{
    //
    // Initializes device clock and peripherals
    //
    Device_init();

    //
    // Initializes PIE and clears PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize GPIO configuration
    //
    Device_initGPIO();

    //
    // Disable all CPU interrupts and clear all CPU interrupt flags.
    //
    DINT;
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initializes the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Set up CPUTimer1, LEDs
    //
    Board_init();

    //
    // Configure FreeRTOS
    //
    FreeRTOS_init();

    //
    // Queue, Mutex and Semaphore instances must be added to queue registry
    // in order to inspect in ROV
    //
    vQueueAddToRegistry( binarySem1Handle, "bsem1" );
    vQueueAddToRegistry( mutex1Handle, "mtx1" );

    //
    // Start timer and FreeRTOS task scheduler
    //
    xTimerStart(myTimer1Handle, pdMS_TO_TICKS(15));
    vTaskStartScheduler();

    //
    // Loop forever. This statement should never be reached.
    //
    while(1)
    {
        ;
    }
}

//
// vApplicationMallocFailedHook - Hook function for catching pvPortMalloc() failures
//
void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

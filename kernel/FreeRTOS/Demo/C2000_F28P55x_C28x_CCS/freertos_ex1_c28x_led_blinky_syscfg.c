//#############################################################################
//
// FILE:   freertos_ex1_c28x_led_blinky_syscfg.c
//
// TITLE:  FreeRTOS C28x LED Blinky (SYSCONFIG)
//
//! This example demonstrates the FreeRTOS specific configuration through
//! Sysconfig. The example configures LEDs, CPUTimers and FreeRTOS and creates
//! two tasks which blinks the on-board LEDs periodically.
//!
//! External Connections:
//! None
//!
//! Watch Variables:
//!  - None - The on-board LEDs should be blinking periodically
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.co/
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

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "FreeRTOS.h"
#include "board.h"
#include "c2000_freertos.h"

#define RED         0xDEADBEAF
#define BLUE        0xBAADF00D

//
// Function Prototypes
//
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationMallocFailedHook( void );

//
// Timer1 ISR
//
__interrupt void timer1_ISR(void);

//
// Task Functions
//
void LED_TaskRed(void * pvParameters);
void LED_TaskBlue(void * pvParameters);
static void blueLedToggle(void);
static void redLedToggle(void);
static void ledToggle(uint32_t led);

//
// Main
//
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
    // Set up CPUTimer1, LEDs.
    //
    Board_init();

    //
    // Configure FreeRTOS
    //
    FreeRTOS_init();

    //
    // Loop forever. This statement should never be reached.
    //
    while(1)
    {
    }
}

//
// Timer1 ISR - Gives semaphore for RED task
//
__interrupt void timer1_ISR( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR( binarySem1Handle, &xHigherPriorityTaskWoken );

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//
// LED_TaskRed - Takes Semaphore and then toggles the LED
//
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

//
// LED_TaskBlue - Toggles LED and blocks for 250 ticks
//
void LED_TaskBlue(void * pvParameters)
{
    for(;;)
    {
        ledToggle((uint32_t)pvParameters);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

//
// Helper functions
//
static void blueLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, counter & 1);
}

static void redLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_writePin(DEVICE_GPIO_PIN_LED2, counter & 1);
}

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

//
// vApplicationStackOverflowHook - Checks run time stack overflow
//
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
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



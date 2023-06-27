/*
 * Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
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
 */

//#############################################################################
//
// FILE:   freertos_ex1_led_blink_cm.c
//
// TITLE:  FreeRTOS based LED Blinky Example
//
//! \addtogroup driver_cm_c28x_dual_example_list
//! <h1> FreeRTOS based LED Blinky Example (CM)</h1>
//!
//! This example demonstrates how to blink a LED through freeRTOS tasks in CM.
//! The CPU1 project passes the ownership of GPIOs connected to LED1 and LED2
//! to CM. The CM tasks toggles LED1 and LED2 alternatively at fixed frequency.
//!
//! \b External \b Connections \n
//!  - None.
//!
//! \b Watch \b Variables \n
//!  - None.
//!
//

//
// Included Files
//
#include "driverlib_cm.h"
#include "cm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//
// Defines
//

//
// Macro to define stack size of individual tasks
//
#define LED_TASK1_STACK_SIZE 100
#define LED_TASK2_STACK_SIZE 100

//
// Macro to define priorities of individual tasks
//
#define LED_TASK1_PRIORITY   1
#define LED_TASK2_PRIORITY   1

//
// Macro to define delay(in ticks) between LED toggle
// in tasks. This delay determines the LED toggle
// frequency within a task.
// Note: 1 tick is configured as 1ms in FreeRTOSConfig.h
//
#define LED_TASK1_BLINK_DELAY 500
#define LED_TASK2_BLINK_DELAY 500

//
// Function prototypes
//
void ledTask1Func(void *pvParameters);
void ledTask2Func(void *pvParameters);

//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);

//
// Globals
//
TaskHandle_t ledTask1, ledTask2;

//
// Main
//
void main(void)
{
    //
    // Initialize device clock and peripherals
    //
    CM_init();

    //
    // Initialize GPIO and configure the GPIO pin as a push-pull output
    //
    // This is configured by CPU1

    //
    // Create tasks to toggle LEDs at fixed frequency
    //
    if(xTaskCreate(ledTask1Func, (const portCHAR *)"ledTask1", LED_TASK1_STACK_SIZE, NULL,
                   (tskIDLE_PRIORITY + LED_TASK1_PRIORITY), &ledTask1) != pdTRUE)
    {
        __asm("   bkpt #0");
    }

    if(xTaskCreate(ledTask2Func, (const portCHAR *)"ledTask2", LED_TASK2_STACK_SIZE, NULL,
                   (tskIDLE_PRIORITY + LED_TASK2_PRIORITY), &ledTask2) != pdTRUE)
    {
        __asm("   bkpt #0");
    }

    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, loop forever.
    //
    for(;;)
    {
    }
}

//
// led Task1 Func - Task function for led task 1
//
void ledTask1Func(void *pvParameters)
{
    while(1)
    {
        //
        // Toggle LED2 for the entire task duration
        //
        GPIO_togglePin(DEVICE_GPIO_PIN_LED1);

        //
        // Delay until LED_TASK1_BLINK_DELAY
        //
        vTaskDelay(LED_TASK1_BLINK_DELAY);

    }
}

//
// led Task2 Func - Task function for led task 2
//
void ledTask2Func(void *pvParameters)
{
    while(1)
    {

        //
        // Toggle LED2 for the entire task duration
        //
        GPIO_togglePin(DEVICE_GPIO_PIN_LED2);

        //
        // Delay until LED_TASK2_BLINK_DELAY ticks
        //
        vTaskDelay(LED_TASK2_BLINK_DELAY);
    }
}

//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//
// End of File
//

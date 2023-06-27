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
// FILE:   freertos_ex3_uart_i2c_cm.c
//
// TITLE:  FreeRTOS based UART and I2C Demo Example
//
//! \addtogroup driver_cm_c28x_dual_example_list
//! <h1> FreeRTOS based UART and I2C Demo Example (CM)</h1>
//!
//! This example demonstrates FreeRTOS usage on CM core. The example
//! demonstrates communication across c28x and CM through I2C and UART
//! modules with CM demonstrating usage of FreeRTOS to configure transfers.
//! CM configures multiple tasks for blinking LED, I2C and UART transfers.
//!
//! CM-I2C0 is configured as master and sends data to C28x I2CB which then
//! prints the received data on terminal via SCIA and sends the data back
//! to CM-UART0 via SCIB. Data received by CM-UART0 at CM side is passed to
//! I2C task through a queue, where the correctness of data is checked and
//! next byte of data is sent to C28x via I2C0.
//!
//! A terminal such as 'putty' can be used to view the data received at
//!  at C28x side via I2CB. Open a COM port with the following settings
//!  using a terminal:
//!  -  Find correct COM port
//!  -  Bits per second = 9600
//!  -  Data Bits = 8
//!  -  Parity = None
//!  -  Stop Bits = 1
//!  -  Hardware Control = None
//!
//! \b External \b Connections \n
//!  - Connect the SCI-A port to a PC via a transceiver and cable.
//!  - I2CB SDA(GPIO40)(89) - CM I2C0 SDA(GPIO31)(82)(I2C specific connections)
//!  - I2CB SCL(GPIO41)(91) - CM I2C0 SCL(GPIO32)(85 (I2C specific connections)
//!  - SCIB Tx(GPIO18)(71)  - UART0 Rx(GPIO85)(152)(UART specific connections)
//!
//! \b Watch \b Variables \n
//!  - i2cTxData  - Data transmitted through CM-I2C0.
//!  - uartRxData - Data received by CM-UART0.
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
#define SLAVE_ADDRESS   0x3C

//
// Macro to define stack size of individual tasks
//
#define LED_TASK1_STACK_SIZE 100
#define I2C_TASK1_STACK_SIZE 100

//
// Macro to define priorities of individual tasks
//
#define LED_TASK1_PRIORITY   1
#define I2C_TASK1_PRIORITY   1

//
// Macro to define delay(in ticks) for LED and
// I2C tasks
// Note: 1 tick is configured as 1ms in FreeRTOSConfig.h
//
#define LED_TASK1_DELAY 500
#define I2C_TASK1_DELAY 500

//
// Globals
//
uint16_t i2cTxData;
uint16_t queueMsg;
uint32_t intStatusVar;
uint16_t uartRxData;

//
// Task handles
//
TaskHandle_t ledTask1, i2cTask1;

//
// The queue used to send message from UART ISR to I2C task
//
xQueueHandle i2cTaskQueue;

//
// Function prototypes
//
void ledTask1Func(void *pvParameters);
void i2cTask1Func(void *pvParameters);
void configCMI2C0(void);
void configCMUART0(void);

//
// UART ISR
//
__interrupt void UART0RxISR(void);

//
// This hook is called by FreeRTOS when stack overflow error is detected.
//
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);

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
    // Configure UARTo interrupt
    //
    Interrupt_enable(INT_UART0);
    Interrupt_registerHandler(INT_UART0, UART0RxISR);
    Interrupt_enableInProcessor();

    //
    // Configure I2C to transmit data as master
    //
    configCMI2C0();

    //
    // Configure UART to receive data
    //
    configCMUART0();

    //
    // Initialize the data to be sent over I2C
    //
    i2cTxData = 0;

    //
    // Initialize GPIO and configure the GPIO pin as a push-pull output
    //
    // This is configured by CPU1

    //
    // Create queue to exchange data between UART ISR and I2C task
    //
    i2cTaskQueue = xQueueCreate(10, sizeof(uint16_t));
    if(i2cTaskQueue == NULL)
    {
        __asm("   bkpt #0");
    }

    //
    // Create tasks to toggle LEDs at varying frequency
    //
    if(xTaskCreate(ledTask1Func, (const portCHAR *)"ledTask1", LED_TASK1_STACK_SIZE, NULL,
                   (tskIDLE_PRIORITY + LED_TASK1_PRIORITY), &ledTask1) != pdTRUE)
    {
        __asm("   bkpt #0");
    }

    if(xTaskCreate(i2cTask1Func, (const portCHAR *)"i2cTask1", I2C_TASK1_STACK_SIZE, NULL,
                       (tskIDLE_PRIORITY + I2C_TASK1_PRIORITY), &i2cTask1) != pdTRUE)
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
        GPIO_togglePin(DEVICE_GPIO_PIN_LED2);

        //
        // Delay until LED_TASK1_BLINK_DELAY
        //
        vTaskDelay(LED_TASK1_DELAY);
    }
}

//
// i2c Task1 Func - Task function for i2c task 1
//
void i2cTask1Func(void *pvParameters)
{
    //
    // Send first data
    //
    i2cTxData = 1;

    // Place the data to be sent in the data register.
    //
    I2C_putMasterData(I2C0_BASE, i2cTxData);

    //
    // Initiate send of single piece of data from the master.  Since the
    // loopback mode is enabled, the Master and Slave units are connected
    // allowing us to receive the same data that we sent out.
    //
    I2C_setMasterConfig(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    while(1)
    {
        //
        // Check the data received over UART
        //
        if(xQueueReceive(i2cTaskQueue, &queueMsg, portMAX_DELAY) == pdTRUE)
        {
            if(queueMsg == i2cTxData)
            {
                //
                // Increment the data value
                //
                i2cTxData = ((i2cTxData + 1) % 256U);

                //
                // Place the data to be sent in the data register.
                //
                I2C_putMasterData(I2C0_BASE, i2cTxData);

                //
                // Initiate send of single piece of data from the master.  Since the
                // loopback mode is enabled, the Master and Slave units are connected
                // allowing us to receive the same data that we sent out.
                //
                I2C_setMasterConfig(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
                vTaskDelay(I2C_TASK1_DELAY);
            }
            else
            {
                __asm("   bkpt #0");
            }
        }
    }
}

//
// UART0 Rx ISR- ISR for UART Rx
//
__interrupt void UART0RxISR(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    //
    // Get the interrupt status.
    //
    intStatusVar = UART_getInterruptStatus(UART0_BASE, UART_RAW_INT);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(UART_isDataAvailable(UART0_BASE))
    {
        //
        // Read the next received character from the UART
        //
        uartRxData = (uint16_t)UART_readCharNonBlocking(UART0_BASE);

        //
        // Send message to I2C task
        //
        xQueueSendFromISR(i2cTaskQueue, &uartRxData, &xHigherPriorityTaskWoken);
    }

    //
    // Clear the asserted interrupts.
    //
    UART_clearInterruptStatus(UART0_BASE, intStatusVar);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
// configCMI2C0 - Configure I2C to trasnmit data as master
//
void configCMI2C0(void)
{
    //
    // Enable the Master module.
    //
    I2C_enableMaster(I2C0_BASE);

    //
    // I2C configuration. Set up to transfer data at 100 Kbps.
    //
    I2C_initMaster(I2C0_BASE,I2C_CLK_FREQ,false);

    //
    // Configure the slave address
    //
    I2C_setSlaveAddress(I2C0_BASE,SLAVE_ADDRESS,I2C_MASTER_WRITE);
}

//
// configCMUART0 - Configure UART0 to receive data
//
void configCMUART0(void)
{
    //
    // Configure UART0 to transfer data at 115200 baud.
    //
    UART_setConfig(UART0_BASE, UART_CLK_FREQ , 115200, (UART_CONFIG_WLEN_8 |
                   UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //
    // Disable FIFO to enable 1-byte deep data register
    //
    UART_disableFIFO(UART0_BASE);

    //
    // FIFO interrupt levels are set to generate an interrupt
    // when the TX FIFO(1-byte deep) is less than or equal to
    // 1/8 full and the RX FIFO(1-byte deep) is greater than
    // or equal to 1/8 full.
    //
    UART_setFIFOLevel(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    //
    // Receive interrupt configuration
    //
    UART_clearInterruptStatus(UART0_BASE,UART_INT_RX | UART_INT_RT);
    UART_enableInterrupt(UART0_BASE, UART_INT_RX);
}

//
// End of File
//

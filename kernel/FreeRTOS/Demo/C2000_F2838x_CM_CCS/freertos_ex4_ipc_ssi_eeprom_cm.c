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
// FILE:   freertos_ex4_ipc_ssi_eeprom_cm.c
//
// TITLE:  FreeRTOS based SSI EPPROM example with IPC
//
//! \addtogroup driver_cm_c28x_dual_example_list
//! <h1> FreeRTOS based SSI EPPROM example with IPC (CM)</h1>
//!
//! This example demonstrates FreeRTOS usage on CM core. The example
//! demonstrates interface of SSI with an EEPROM module. This example is
//! written to work with the SPI Serial EEPROM AT25128/256. The C28x core sends
//! IPC commands to the CM core to read and write contents to the EEPROM.
//!
//! This application configures IPC interrupt 1 and an SSI task. Once the
//! tasks are created, it notifies the CPU1 core by sending an IPC command
//! and starts the FreeRTOS scheduler. On receiving the command from the C28x
//! core, IPC_ISR is invoked which sends the message to the SSI task using a
//! queue. SSI task waits for a message in the queue and sends command to
//! the EEPROM module accordingly. LED2 is toggled every time a new message
//! is received by the SSI task.
//! This also includes a LED task which toggles LED1 in a periodic manner.
//!
//! \b External \b Connections \n
//!  - Connect external SPI EEPROM
//!  - Connect GPIO16 (SIMO) to external EEPROM SI pin
//!  - Connect GPIO17 (SOMI) to external EEPROM SO pin
//!  - Connect GPIO18 (CLK) to external EEPROM SCK pin
//!  - Connect GPIO11 (CS) to external EEPROM CS pin
//!  - Connect the external EEPROM VCC and GND pins
//!
//! \b Watch \b Variables \n
//!  - None
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
#define SPI_TASK_STACK_SIZE  100
#define SPI_TASK_PRIORITY    1
#define LED_TASK_STACK_SIZE  100
#define LED_TASK_PRIORITY    1
#define LED_TASK_BLINK_DELAY 500

//
// SPI EEPROM status
//
#define MSG_STATUS_READY_M          0x01 // EEPROM is ready (not busy)
#define MSG_STATUS_WRITE_READY_M    0x02 // EEPROM is ready to write
#define MSG_STATUS_BUSY             0xFF // EEPROM is busy (internal write)

//
// Opcodes for the EEPROM (8-bit)
//
#define RDSR                        0x05  // Read Status Register
#define READ                        0x03  // Read Data from Memory Array
#define WRITE                       0x02  // Write Data to Memory Array
#define WREN                        0x06  // Set Write Enable Latch
#define WRDI                        0x04  // Reset Write Enable Latch
#define WRSR                        0x01  // Write Status Register

//
// IPC commands
//
#define EEPROM_CMD_DATA_WRITE       0x11
#define EEPROM_CMD_DATA_READ        0x22
#define CM_READY                    0xCC

//
// Defines for Chip Select toggle.
//
#define CS_LOW                      GPIO_writePin(11, 0)
#define CS_HIGH                     GPIO_writePin(11, 1)

//
// Function prototypes
//
void ssiTaskFunc(void *pvParameters);
void ledTaskFunc(void *pvParameters);
void IPC_ISR(void);
uint8_t readStatusRegister(void);
void writeData(uint16_t address, uint8_t * data, uint16_t length);
void readData(uint16_t address, uint8_t * data, uint16_t length);
void enableWrite(void);

//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);

//
// Globals
//

//
// Buffer to store the data read from EEPROM
//
#pragma DATA_SECTION(EEPROM_data_read, "MSGRAM_CM_TO_CPU1")
uint8_t EEPROM_data_read[30];

//
// Task handles
//
TaskHandle_t ledTask, ssiTask;

//
// The queue used to send message from IPC_ISR to SSI task
//
xQueueHandle EEPROM_Msg_Queue;

typedef struct
{
    uint8_t  command;
    uint8_t  data_size;
    uint16_t EEPROM_address;
    uint8_t *data_address;
}EEPROM_Msg;

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
    // Enable IPC interrupt
    //
    Interrupt_registerHandler(INT_CPU1TOCMIPC1, IPC_ISR);
    Interrupt_enable(INT_CPU1TOCMIPC1);

    //
    // Create a queue of type EEPROM_Msg and size 10
    //
    EEPROM_Msg_Queue = xQueueCreate(10, sizeof(EEPROM_Msg));
    if(EEPROM_Msg_Queue == NULL)
    {
        __asm("   bkpt #0");
    }

    //
    // Create LED and SSI tasks
    //
    if(xTaskCreate(ledTaskFunc, (const portCHAR *)"ledTask", LED_TASK_STACK_SIZE, NULL,
                   (LED_TASK_PRIORITY), &ledTask) != pdTRUE)
    {
        __asm("   bkpt #0");
    }

    if(xTaskCreate(ssiTaskFunc, (const portCHAR *)"ssiTask", SPI_TASK_STACK_SIZE, NULL,
                       (SPI_TASK_PRIORITY), &ssiTask) != pdTRUE)
    {
        __asm("   bkpt #0");
    }

    //
    // Send a command to C28x core indicating the tasks are created. This also
    // sends the address of the buffer used to store the data read from the
    // EEPROM. The parameter IPC_ADDR_CORRECTION_ENABLE is used so that it gets
    // converted to the C28x space address.
    //
    IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG1, IPC_ADDR_CORRECTION_ENABLE,
                    CM_READY, (uint32_t)EEPROM_data_read, 0);

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
// LED Task Function
//
void ledTaskFunc(void *pvParameters)
{
    while(1)
    {
        //
        // Toggle LED2 for the entire task duration
        //
        GPIO_togglePin(DEVICE_GPIO_PIN_LED1);

        //
        // Delay until LED_TASK_BLINK_DELAY
        //
        vTaskDelay(LED_TASK_BLINK_DELAY);
    }
}

//
// SSI Task Function
//
void ssiTaskFunc(void *pvParameters)
{
    EEPROM_Msg msg;

    //
    // Configure SSI in master mode, baud rate = 100000, dataWidth = 8
    //
    SSI_setConfig(SSI0_BASE, CM_CLK_FREQ, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 8);

    //
    // Enable the SSI0 module.
    //
    SSI_enableModule(SSI0_BASE);

    while(1)
    {
        //
        // Wait for the message from IPC_ISR
        //
        if(xQueueReceive(EEPROM_Msg_Queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            //
            // Toggle an LED indicating the message receive
            //
            GPIO_togglePin(DEVICE_GPIO_PIN_LED2);

            //
            // Wait until the EEPROM is ready
            //
            while((readStatusRegister() & MSG_STATUS_READY_M) == MSG_STATUS_READY_M)
            {
            }

            if(msg.command == EEPROM_CMD_DATA_WRITE)
            {
                //
                // Send command to EEPROM module to enable write
                //
                enableWrite();

                //
                // Wait until the EEPROM is ready to write data
                //
                while((readStatusRegister() & MSG_STATUS_WRITE_READY_M) != MSG_STATUS_WRITE_READY_M)
                {
                }

                //
                // Send command to EEPROM module to write data
                //
                writeData(msg.EEPROM_address, msg.data_address, msg.data_size);
            }

            else if(msg.command == EEPROM_CMD_DATA_READ)
            {
                //
                // Send command to EEPROM module to read data
                //
                readData(msg.EEPROM_address, EEPROM_data_read, msg.data_size);
            }

            else
            {
                //
                // Invalid command
                //
                __asm("   bkpt #0");
            }
        }

        //
        // Notify C28x core the completion of data read/write
        //
        IPC_setFlagLtoR(IPC_CM_L_CPU1_R, IPC_FLAG1);
    }
}

//
// IPC ISR function
//
void IPC_ISR(void)
{
    uint32_t command, addr, data;
    EEPROM_Msg msg;
    BaseType_t xHigherPriorityTaskWoken;

    //
    // Read command from the C28x core
    //
    IPC_readCommand(IPC_CM_L_CPU1_R, IPC_FLAG1, IPC_ADDR_CORRECTION_ENABLE,
                    &command, &addr, &data);

    //
    // Message to be send to the SSI task
    // command parameter indicates whether to read or write to the EEPROM
    // addr parameter contains the address of the data to be written to EEPROM
    // data parameter includes the EEPROM address and size of data
    // The data value is multiplied by 2 since the char size is 16 bits in C28x
    // and 8 bits in CM core
    //
    msg.command = command;
    msg.data_address = (uint8_t *)addr;
    msg.EEPROM_address = data >> 16;
    msg.data_size = data & 0xFF;
    msg.data_size *= 2;

    //
    // Acknowledge the IPC flag
    //
    IPC_ackFlagRtoL(IPC_CM_L_CPU1_R, IPC_FLAG1);

    //
    // Send message to SSI task
    //
    xQueueSendFromISR(EEPROM_Msg_Queue, &msg, &xHigherPriorityTaskWoken);
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
// Function to send RDSR opcode and return the status of the EEPROM
//
uint8_t readStatusRegister(void)
{
    uint32_t temp;

    //
    // Pull chip select low.
    //
    CS_LOW;

    //
    // Send RDSR opcode
    //
    SSI_writeData(SSI0_BASE, RDSR);

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send dummy data to receive the status.
    //
    SSI_writeData(SSI0_BASE, 0x00);

    //
    // Read status register.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Pull chip select high.
    //
    CS_HIGH;

    //
    // Read the status from the receive buffer
    //
    return(temp & 0xFF);
}

//
// Function to send the WREN opcode
//
void enableWrite(void)
{
    uint32_t temp;
    //
    // Pull chip select low.
    //
    CS_LOW;

    //
    // Send the WREN opcode.
    //
    SSI_writeData(SSI0_BASE, WREN);

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Pull chip select high.
    //
    CS_HIGH;
}

//
// Function to write data to the EEPROM
// - address is the byte address of the EEPROM
// - data is a pointer to an array of data being sent
// - length is the number of characters in the array to send
//
void writeData(uint16_t address, uint8_t * data, uint16_t length)
{
    uint16_t i;
    uint32_t temp;

    //
    // Pull chip select low.
    //
    CS_LOW;

    //
    // Send the WRITE opcode.
    //
    SSI_writeData(SSI0_BASE, WRITE);

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send the MSB of the address of the EEPROM.
    //
    SSI_writeData(SSI0_BASE, (address >> 8));

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send the LSB of the address to the EEPROM.
    //
    SSI_writeData(SSI0_BASE, address & 0xFF);

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send the data.
    //
    for(i = 0; i < length; i++)
    {
        //
        // Send the data.
        //
        SSI_writeData(SSI0_BASE, data[i]);

        //
        // Dummy read to clear INT_FLAG.
        //
        SSI_readData(SSI0_BASE, &temp);
    }

    //
    // Pull chip select high.
    //
    CS_HIGH;
}

//
// Function to read data from the EEPROM
// - address is the byte address of the EEPROM
// - data is a pointer to an array of data being received
// - length is the number of characters in the array to receive
//
void readData(uint16_t address, uint8_t * data, uint16_t length)
{
    uint16_t i;
    uint32_t temp;

    CS_LOW;

    //
    // Send the READ opcode
    //
    SSI_writeData(SSI0_BASE, READ);

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send the MSB of the address of the EEPROM
    //
    SSI_writeData(SSI0_BASE, (address >> 8));

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Send the LSB of the address of the EEPROM
    //
    SSI_writeData(SSI0_BASE, (address & 0xFF));

    //
    // Dummy read to clear INT_FLAG.
    //
    SSI_readData(SSI0_BASE, &temp);

    //
    // Read the data from the EEPROM
    //
    for(i = 0; i < length; i++)
    {
        //
        // Send dummy data to receive the EEPROM data
        //
        SSI_writeData(SSI0_BASE, 0x00);
        SSI_readData(SSI0_BASE, &temp);
        data[i] = temp & 0xFF;
    }

    CS_HIGH;
}

//
// End of File
//

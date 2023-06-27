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
// FILE:   freertos_ex4_ipc_ssi_eeprom_cpu1.c.c
//
// TITLE:  FreeRTOS based SSI EPPROM example with IPC
//
//! \addtogroup driver_cm_c28x_dual_example_list
//! <h1> FreeRTOS based SSI EPPROM example with IPC (C28x)</h1>
//!
//! This example demonstrates FreeRTOS usage on CM core. The example
//! demonstrates interface of SSI with an EEPROM module. This example is
//! written to work with the SPI Serial EEPROM AT25128/256. The C28x core sends
//! IPC commands to the CM core to read and write contents to the EEPROM.
//!
//! This application configures the GPIOs needed for SSI (CM core) and SCI
//! (C28x core) modules. It sends IPC commands to the CM core to read and write
//! contents to the EEPROM module.
//! It also sends the data read and written to the EEPROM via SCI to the serial
//! COM port
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
//!
//

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include <string.h>

//
// Defines
//
#define EEPROM_ADDR                 0x0000

//
// IPC commands
//
#define EEPROM_CMD_DATA_WRITE       0x11
#define EEPROM_CMD_DATA_READ        0x22
#define CM_READY                    0xCC

//
// Function prototypes
//
void configExPins();
void configC28xSCITerminal(void);

//
// Globals
//
#pragma DATA_SECTION(EEPROM_data_write, "MSGRAM_CPU_TO_CM")
char EEPROM_data_write[30];

//
// Main
//
void main(void)
{
    uint32_t command, addr, data;
    char *EEPROM_data_read;
    unsigned char *msg;

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Boot CM core
    //
#ifdef _FLASH
    Device_bootCM(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    Device_bootCM(BOOTMODE_BOOT_TO_S0RAM);
#endif

    //
    // Initialize GPIO and configure the GPIO pin as a push-pull output
    //
    Device_initGPIO();

    //
    // Configure example specific GPIO pins
    //
    configExPins();

    //
    // Configure the SCI module
    //
    configC28xSCITerminal();

    //
    // Wait for the message from the CM core. The message includes the address
    // of buffer used to store the data read from the EEPROM module
    //
    IPC_waitForFlag(IPC_CPU1_L_CM_R, IPC_FLAG1);
    IPC_readCommand(IPC_CPU1_L_CM_R, IPC_FLAG1, IPC_ADDR_CORRECTION_ENABLE,
                    &command, &addr, &data);
    IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG1);

    if(command != CM_READY)
    {
        ESTOP0;
    }
    EEPROM_data_read = (char *)addr;


    while(1)
    {
        //
        // Write some data in EEPROM
        //
        strcpy(EEPROM_data_write, "Hello World");
        IPC_sendCommand(IPC_CPU1_L_CM_R,
                        IPC_FLAG1,
                        IPC_ADDR_CORRECTION_ENABLE,
                        EEPROM_CMD_DATA_WRITE,       // command
                        (uint32_t)EEPROM_data_write, // address of the data buffer
                        (EEPROM_ADDR << 8) | 11);    // EEPROM address and data size

        //
        // Wait for completion
        //
        IPC_waitForFlag(IPC_CPU1_L_CM_R, IPC_FLAG1);
        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG1);

        //
        // Read back the data
        //
        IPC_sendCommand(IPC_CPU1_L_CM_R,
                        IPC_FLAG1,
                        IPC_ADDR_CORRECTION_ENABLE,
                        EEPROM_CMD_DATA_READ,      // command
                        0,                         // address not used
                        (EEPROM_ADDR << 8) | 11);  // EEPROM address and data size

        //
        // Wait for completion
        //
        IPC_waitForFlag(IPC_CPU1_L_CM_R, IPC_FLAG1);
        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG1);

        //
        // Print the data on the SCI terminal
        //
        msg = "\r\nData written to EEPROM : ";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 27);
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)EEPROM_data_write, 11);

        msg = "\r\nData read from EEPROM : ";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 26);
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)EEPROM_data_read, 11);

        //
        // Wait for some time
        //
        DEVICE_DELAY_US(1000000);

        //
        // Write some data in EEPROM
        //
        strcpy(EEPROM_data_write, "Texas Instruments");
        IPC_sendCommand(IPC_CPU1_L_CM_R,
                        IPC_FLAG1,
                        IPC_ADDR_CORRECTION_ENABLE,
                        EEPROM_CMD_DATA_WRITE,       // command
                        (uint32_t)EEPROM_data_write, // address of the data buffer
                        (EEPROM_ADDR << 8) | 17);    // EEPROM address and data size
        //
        // Wait for completion
        //
        IPC_waitForFlag(IPC_CPU1_L_CM_R, IPC_FLAG1);
        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG1);

        //
        // Read back the data
        //
        IPC_sendCommand(IPC_CPU1_L_CM_R,
                        IPC_FLAG1,
                        IPC_ADDR_CORRECTION_ENABLE,
                        EEPROM_CMD_DATA_READ,       // command
                        0,                          // address not used
                        (EEPROM_ADDR << 8) | 17);   // EEPROM address and data size

        //
        // Wait for completion
        //
        IPC_waitForFlag(IPC_CPU1_L_CM_R, IPC_FLAG1);
        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG1);

        //
        // Print the data on the SCI terminal
        //
        msg = "\r\nData written to EEPROM : ";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 27);
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)EEPROM_data_write, 17);

        msg = "\r\nData read from EEPROM : ";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 26);
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)EEPROM_data_read, 17);

        //
        // Wait for some time
        //
        DEVICE_DELAY_US(1000000);
    }
}

//
// configExPins - Example specific pin configuration
//
void configExPins(void)
{
    //
    // C28x side GPIO configurations
    //

    //
    // UART configuration on C28x side for terminal display
    // GPIO28 is the SCI Rx pin. GPIO29 is the SCI Tx pin.
    //
    GPIO_setMasterCore(28, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_28_SCIA_RX);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);

    GPIO_setMasterCore(29, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_29_SCIA_TX);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(29, GPIO_QUAL_ASYNC);


    //
    // CM side GPIO configurations
    //

    //
    // LED specific configurations
    //
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED1, GPIO_CORE_CM);

    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED2, GPIO_CORE_CM);


    //
    // SSI specific pin configuration on CM side
    //
    //
    // GPIO16 is the SPISIMOA clock pin.
    //
    GPIO_setPinConfig(GPIO_16_SSIA_TX);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(16, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(16, GPIO_CORE_CM);

    //
    // GPIO17 is the SPISOMIA.
    //
    GPIO_setPinConfig(GPIO_17_SSIA_RX);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(17, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(17, GPIO_CORE_CM);

    //
    // GPIO18 is the SPICLKA.
    //
    GPIO_setPinConfig(GPIO_18_SSIA_CLK);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(18, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(18, GPIO_CORE_CM);

    //
    // GPIO11 is the SPICS.
    //
    GPIO_setPinConfig(GPIO_11_GPIO11);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(11, GPIO_QUAL_ASYNC);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(11, GPIO_CORE_CM);

}
void configC28xSCITerminal(void)
{
    unsigned char *msg;

    //
    // Initialize SCIA and its FIFO.
    //
    SCI_performSoftwareReset(SCIA_BASE);

    //
    // Configure SCIA module
    //
    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 9600, (SCI_CONFIG_WLEN_8 |
                                                        SCI_CONFIG_STOP_ONE |
                                                        SCI_CONFIG_PAR_NONE));
    SCI_resetChannels(SCIA_BASE);
    SCI_resetRxFIFO(SCIA_BASE);
    SCI_resetTxFIFO(SCIA_BASE);
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(SCIA_BASE);
    SCI_enableModule(SCIA_BASE);
    SCI_performSoftwareReset(SCIA_BASE);

#ifdef AUTOBAUD
    //
    // Perform an autobaud lock.
    // SCI expects an 'a' or 'A' to lock the baud rate.
    //
    SCI_lockAutobaud(SCIA_BASE);
#endif

    //
    // Send starting message.
    //
    msg = "\r\nWelcome to FreeRTOS Demo!\0";
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 28);
}

//
// End of File
//

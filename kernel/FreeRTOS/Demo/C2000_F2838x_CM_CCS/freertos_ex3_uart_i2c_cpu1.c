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
// FILE:   freertos_ex3_uart_i2c_cpu1.c
//
// TITLE:  FreeRTOS based UART and I2C Demo Example
//
//! \addtogroup driver_cm_c28x_dual_example_list
//! <h1> FreeRTOS based UART and I2C Demo Example (C28x)</h1>
//!
//! This example demonstrates FreeRTOS usage on CM core. The example
//! demonstrates communication across C28x and CM through I2C and UART
//! modules with CM demonstrating usage of FreeRTOS to configure transfers.
//! CM configures multiple tasks for blinking LED, I2C and UART transfers.
//!
//! CM-I2C0 is configured as master and sends data to C28x I2CB which then
//! prints the received data on terminal via SCIA and sends the data back
//! to CM-UART0 via SCIB. Data received by CM-UART0 at CM side is passed to
//! I2C task through a queue, where the correctness of data is checked and
//! next byte of data is sent to C28x via I2C0.
//!
//!  A terminal such as 'putty' can be used to view the data received at
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
//!  - i2cbRxData - Data received at C28x I2CB side.
//!
//

//
// Included Files
//
#include "driverlib.h"
#include "device.h"

//
// Defines
//

//
// I2C slave address
//
#define SLAVE_ADDRESS   0x3C

//
// Terminal specific msg variables
//
unsigned char *msg;

//
// Received data at I2CB
//
uint16_t i2cbRxData;

//
// Function Prototypes
//
void configExPins(void);
void configC28xI2CB(void);
void configC28xSCIB(void);
void configC28xSCITerminal(void);

//
// I2CB ISR
//
__interrupt void i2cFIFOISR(void);

//
// Main
//
void main(void)
{
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
    // Configure I2CB as slave to receive data from master CM-I2C0
    //
    configC28xI2CB();

    //
    // Configure SCIB to transmit data to CM-UART0
    //
    configC28xSCIB();

    //
    // Configure SCIA to display received messages on terminal
    //
    configC28xSCITerminal();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Register handler and enable I2C FIFO ISR interrupt
    //
    Interrupt_register(INT_I2CB_FIFO, &i2cFIFOISR);
    Interrupt_enable(INT_I2CB_FIFO);

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    //
    // Loop Forever
    //
    for(;;)
    {
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
    // I2C specific pin configuration on C28x side. Initialize
    // GPIOs 40 and 41 for use as SDA B and SCL B respectively
    //
    GPIO_setPinConfig(GPIO_40_I2CB_SDA);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(40, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_41_I2CB_SCL);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(41, GPIO_QUAL_ASYNC);

    //
    // UART specific pin configuration on C28x side.
    // GPIO19 is the SCI Rx pin. GPIO18 is the SCI Tx pin.
    //
    GPIO_setMasterCore(19, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_19_SCIB_RX);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(19, GPIO_QUAL_ASYNC);

    GPIO_setMasterCore(18, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_18_SCIB_TX);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(18, GPIO_QUAL_ASYNC);

    //
    // CM side GPIO configurations
    //

    //
    // LED specific configurations
    //
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED2, GPIO_CORE_CM);

    //
    // I2C specific pin configuration on CM side
    //
    GPIO_setPinConfig(GPIO_31_CM_I2CA_SDA);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(31, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(31, GPIO_CORE_CM);

    GPIO_setPinConfig(GPIO_32_CM_I2CA_SCL);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(32, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(32, GPIO_CORE_CM);

    //
    // UART specific pin configuration on CM side
    // GPIO85 is the SCI Rx pin. GPIO84 is the SCI Tx pin.
    //
    GPIO_setPinConfig(GPIO_85_UARTA_RX);
    GPIO_setDirectionMode(85, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(85, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(85, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(85, GPIO_CORE_CM);

    GPIO_setPinConfig(GPIO_84_UARTA_TX);
    GPIO_setDirectionMode(84, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(84, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(84, GPIO_QUAL_ASYNC);
    GPIO_setMasterCore(84, GPIO_CORE_CM);
}

//
// configC28xI2CB - Configuring C28x I2CB module as slave
//
void configC28xI2CB(void)
{
    I2C_disableModule(I2CB_BASE);

    //
    // I2C slave configuration
    //
    I2C_setConfig(I2CB_BASE, I2C_SLAVE_RECEIVE_MODE);

    I2C_setDataCount(I2CB_BASE, 1);
    I2C_setBitCount(I2CB_BASE, I2C_BITCOUNT_8);

    I2C_setOwnSlaveAddress(I2CB_BASE, SLAVE_ADDRESS);
    I2C_setEmulationMode(I2CB_BASE, I2C_EMULATION_FREE_RUN);

    I2C_enableFIFO(I2CB_BASE);
    I2C_clearInterruptStatus(I2CB_BASE, I2C_INT_RXFF);

    //
    // Receive FIFO interrupt levels are set to generate an interrupt
    // when the 16 byte RX fifo contains 2 or greater bytes of data.
    //
    I2C_setFIFOInterruptLevel(I2CB_BASE, I2C_FIFO_TX2, I2C_FIFO_RX1);
    I2C_enableInterrupt(I2CB_BASE, I2C_INT_RXFF | I2C_INT_STOP_CONDITION);

    I2C_enableModule(I2CB_BASE);
}

//
// configC28xSCIB - Configuring C28x SCIB module
//
void configC28xSCIB(void)
{
    //
    // Initialize SCIB and its FIFO.
    //
    SCI_performSoftwareReset(SCIB_BASE);

    //
    // Configure SCIB for 115200 baud rate.
    //
    SCI_setConfig(SCIB_BASE, DEVICE_LSPCLK_FREQ, 115200, (SCI_CONFIG_WLEN_8 |
                                                        SCI_CONFIG_STOP_ONE |
                                                        SCI_CONFIG_PAR_NONE));
    SCI_resetChannels(SCIB_BASE);
    SCI_clearInterruptStatus(SCIB_BASE, SCI_INT_TXFF);
    SCI_enableModule(SCIB_BASE);
    SCI_performSoftwareReset(SCIB_BASE);

#ifdef AUTOBAUD
    //
    // Perform an autobaud lock.
    // SCI expects an 'a' or 'A' to lock the baud rate.
    //
    SCI_lockAutobaud(SCIB_BASE);
#endif
}

//
// configC28xSCITerminal - Configuring C28x SCIA for Terminal Display
//
void configC28xSCITerminal(void)
{
    //
    // Initialize SCIA and its FIFO.
    //
    SCI_performSoftwareReset(SCIA_BASE);

    //
    // Configure SCIA for terminal display.
    //
    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 9600, (SCI_CONFIG_WLEN_8   |
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
    msg = "\r\nWelcome to FreeRTOS Demo!\n";
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 28);
    msg = "\r\nDetails of Data sent from CM side tasks will be displayed here!";
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 66);
}

//
// I2C TX and Receive FIFO ISR
//
 __interrupt void i2cFIFOISR(void)
{
    //
    // If receive FIFO interrupt flag is set, read data
    //
    if((I2C_getInterruptStatus(I2CB_BASE) & I2C_INT_RXFF) != 0)
    {
        i2cbRxData = I2C_getData(I2CB_BASE);

        //
        // Display received data on terminal
        //
        msg = "\r\nMessage received at I2CB:";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 28);
        SCI_writeCharBlockingFIFO(SCIA_BASE, i2cbRxData);

        //
        // Send data received through I2CB back to CM through SCIB
        //
        SCI_writeCharBlockingNonFIFO(SCIB_BASE, i2cbRxData);

        //
        // Clear interrupt flag
        //
        I2C_clearInterruptStatus(I2CB_BASE, I2C_INT_RXFF);
    }

    //
    // Issue ACK
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}
//
// End of File
//

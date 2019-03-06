/*******************************************************************************

interfaces.cpp
(c) 2010 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "interfaces.h"

//Serial pc(USBTX, USBRX); // tx, rx
Serial          pc(USBTX, USBRX);

// A timer for timing how long things take to happen
Timer timer;
// A timer for checking that timeout periods aren't exceeded
Timer timeout;

#define SD_MOSI PTD2
#define SD_MISO PTD3
#define SD_SCLK PTD1
#define SD_CS   PTD0


// Need to create this to be able to read and write files on the mbed 'disk'
// SDFileSystem sdard(SD_MOSI, SD_MISO, SD_SCLK, SD_CS, "sdcard");

// Global variables (default to BDM1 - Master)
DigitalInOut    PIN_BERR(PTB11);     // double bus fault input - will be an input when it is working properly
DigitalInOut    PIN_BKPT(PTE5);      // breakpoint/serial clock
DigitalIn       PIN_FREEZE(PTB10);   // freeze signal
DigitalInOut    PIN_RESET(PTE4);     // reset signal
DigitalOut    PIN_DSI(PTE3);       // data input (to ECU) signal
DigitalIn       PIN_PWR(PTB0);       // power supply
DigitalIn       PIN_DSO(PTE2);       // data output (from ECU) signal


// MASTER BDM1 Connector Pinout
DigitalInOut    BDM1_PIN_BERR(PTB11);     // double bus fault input - will be an input when it is working properly
DigitalInOut    BDM1_PIN_BKPT(PTE5);      // breakpoint/serial clock
DigitalIn       BDM1_PIN_FREEZE(PTB10);   // freeze signal
DigitalInOut    BDM1_PIN_RESET(PTE4);     // reset signal
DigitalOut    BDM1_PIN_DSI(PTE3);       // data input (to ECU) signal
DigitalIn       BDM1_PIN_PWR(PTB0);       // power supply
DigitalIn       BDM1_PIN_DSO(PTE2);       // data output (from ECU) signal

// SLAVE BDM2 Connector Pinout
DigitalInOut    BDM2_PIN_BERR(PTB9);     // double bus fault input - will be an input when it is working properly
DigitalInOut    BDM2_PIN_BKPT(PTE29);      // breakpoint/serial clock
DigitalIn       BDM2_PIN_FREEZE(PTB8);   // freeze signal
DigitalInOut    BDM2_PIN_RESET(PTE23);     // reset signal
DigitalOut    BDM2_PIN_DSI(PTE22);       // data input (to ECU) signal
DigitalIn       BDM2_PIN_PWR(PTB2);       // power supply
DigitalIn       BDM2_PIN_DSO(PTE21);       // data output (from ECU) signal


//LEDS

// Use the LEDs to if anything is happening

DigitalOut      led1(LED_GREEN); // BDM Activity
DigitalOut      led2(LED_BLUE); // Error

Ticker ticker;

void leds_off() {
    ACTIVITYLEDOFF;
}
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
SDFileSystem sdard(SD_MOSI, SD_MISO, SD_SCLK, SD_CS, "sdcard");

// MASTER BDM1 Connector Pinout
DigitalInOut    PIN_BERR(PTB11);     // double bus fault input - will be an input when it is working properly
DigitalInOut    PIN_BKPT(PTE5);      // breakpoint/serial clock
DigitalIn       PIN_FREEZE(PTB10);   // freeze signal
DigitalInOut    PIN_RESET(PTE4);     // reset signal
DigitalInOut    PIN_DSI(PTE3);               // data input (to ECU) signal
DigitalIn       PIN_PWR(PTB0);               // power supply
DigitalIn       PIN_DSO(PTE2);               // data output (from ECU) signal

// DigitalIn       PIN_NC(PTA1);                // connection signal
// DigitalIn       PIN_DS(p27);                // data strobe signal (not used)

//LEDS

// Use the LEDs to if anything is happening

DigitalOut      led1(LED_GREEN); // BDM Activity
DigitalOut      led2(LED_BLUE); // Error

Ticker ticker;

void leds_off() {
    ACTIVITYLEDOFF;
}
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

// We use CAN on mbed pins 29(CAN_TXD) and 30(CAN_RXD).
CAN can(p30, p29);
// CAN_RS pin at Philips PCA82C250 can bus controller.
// activate transceiver by pulling this pin to GND.
// (Rise and fall slope controlled by resistor R_s)
// (+5V result in tranceiver standby mode)
// For further information see datasheet page 4
DigitalOut can_rs_pin(p28);

// Need to create this to be able to read and write files on the mbed 'disk'
SDFileSystem sdard("sdcard");


DigitalIn       PIN_PWR(p19);               // power supply
DigitalIn       PIN_NC(p20);                // connection signal
DigitalInOut    PIN_BERR(p21);              // double bus fault input - will be an input when it is working properly
DigitalInOut    PIN_BKPT(p22);              // breakpoint/serial clock
DigitalInOut    PIN_RESET(p23);             // reset signal
DigitalInOut    PIN_DSI(p24);               // data input (to ECU) signal
DigitalIn       PIN_DSO(p25);               // data output (from ECU) signal
DigitalIn       PIN_FREEZE(p26);            // freeze signal
//DigitalIn       PIN_DS(p27);                // data strobe signal (not used)

//LEDS

// Use the LEDs to if anything is happening

DigitalOut      led1(LED1);                 // LED1 CAN send
DigitalOut      led2(LED2);                 // LED2 CAN receive
DigitalOut      led3(LED3);                 // BDM activity LED
DigitalOut      led4(LED4);                 // Error LED

Ticker ticker;

void leds_off() {
    CANTXLEDOFF;
    CANRXLEDOFF;
    ACTIVITYLEDOFF;
}
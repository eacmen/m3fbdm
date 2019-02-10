/*******************************************************************************

interfaces.cpp - information and definitions about Just4Trionic external interfaces
(c) 2010 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __INTERFACES_H__
#define __INTERFACES_H__

#include "mbed.h"
#include "SDFileSystem.h"
#include "CAN.h"

extern Serial           pc;                     //Serial pc(USBTX, USBRX); // tx, rx

extern SDFileSystem     sdcard;                  

extern Timer            timer;
extern Timer            timeout;

extern DigitalIn        PIN_PWR;                // power supply
// extern DigitalIn        PIN_NC;                 // connection signal
extern DigitalInOut     PIN_BERR;               // double bus fault input - will be an input when it is working properly
extern DigitalInOut     PIN_BKPT;               // breakpoint/serial clock
extern DigitalInOut     PIN_RESET;              // reset signal
extern DigitalInOut     PIN_DSI;                // data input (to ECU) signal
extern DigitalIn        PIN_DSO;                // data output (from ECU) signal
extern DigitalIn        PIN_FREEZE;             // freeze signal
// extern DigitalIn        PIN_DS;                 // data strobe signal (not used)

//LEDS

// Use the LEDs to see if anything is happening
extern DigitalOut       led1;                   // BDM activity LE
extern DigitalOut       led2;                   // Error LED

extern Ticker           ticker;

void leds_off(void);

// led control macros
// These macros use the fastio register method to control the leds
#define ACTIVITYLEDON   led1 = 0   // Turn on the activity led
#define ACTIVITYLEDOFF  led1 = 1   // Turn off the activity led
#define ERRORLEDON      led2 = 0   // Turn on the ERROR led
#define ERRORLEDOFF     led2 = 1   // Turn off the ERROR led

#endif          // __INTERFACES_H__
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

extern CAN              can;                    
extern DigitalOut       can_rs_pin;

extern SDFileSystem     sdcard;                  

extern Timer            timer;
extern Timer            timeout;

extern DigitalIn        PIN_PWR;                // power supply
extern DigitalIn        PIN_NC;                 // connection signal
extern DigitalInOut     PIN_BERR;               // double bus fault input - will be an input when it is working properly
extern DigitalInOut     PIN_BKPT;               // breakpoint/serial clock
extern DigitalInOut     PIN_RESET;              // reset signal
extern DigitalInOut     PIN_DSI;                // data input (to ECU) signal
extern DigitalIn        PIN_DSO;                // data output (from ECU) signal
extern DigitalIn        PIN_FREEZE;             // freeze signal
//extern DigitalIn        PIN_DS;                 // data strobe signal (not used)

//LEDS

// Use the LEDs to see if anything is happening
extern DigitalOut       led1;                   // LED1 CAN send
extern DigitalOut       led2;                   // LED2 CAN receive
extern DigitalOut       led3;                   // BDM activity LE
extern DigitalOut       led4;                   // Error LED

extern Ticker           ticker;

void leds_off(void);

// led control macros
// These macros use the fastio register method to control the leds
#define CANTXLEDON      LPC_GPIO1->FIOSET = (1 << 18)   // Turn on the CAN bus transmitter activity led (led1, P1.18)
#define CANTXLEDOFF     LPC_GPIO1->FIOCLR = (1 << 18)   // Turn off the CAN bus transmitter activity led (led1, P1.18)
#define CANRXLEDON      LPC_GPIO1->FIOSET = (1 << 20)   // Turn on the CAN bus receiver activity led (led2, P1.20)
#define CANRXLEDOFF     LPC_GPIO1->FIOCLR = (1 << 20)   // Turn off the CAN bus receiver activity led (led2, P1.20)
#define ACTIVITYLEDON   LPC_GPIO1->FIOSET = (1 << 21)   // Turn on the activity led (led3, P1.21)
#define ACTIVITYLEDOFF  LPC_GPIO1->FIOCLR = (1 << 21)   // Turn off the activity led (led3, P1.21)
#define ERRORLEDON      LPC_GPIO1->FIOSET = (1 << 23)   // Turn on the ERROR led (led3, P1.23)
#define ERRORLEDOFF     LPC_GPIO1->FIOCLR = (1 << 23)   // Turn off the ERROR led (led3, P1.23)

#endif          // __INTERFACES_H__
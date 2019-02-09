/*******************************************************************************

t8can.h - information and definitions needed for doing things with the T7 ECU
(c) 2010,2011,2012 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __T8CAN_H__
#define __T8CAN_H__

#include "mbed.h"
#include "CAN.h"

#include "common.h"
#include "strings.h"
#include "t8utils.h"
#include "srecutils.h"


extern void t8_can();

void t8_can_show_help();
void t8_can_show_full_help();
uint8_t execute_t8_cmd();


#endif
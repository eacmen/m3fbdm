/*******************************************************************************

canutils.h - information and definitions needed for the CAN bus
(c) 2010, 2012 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __CANUTILS_H__
#define __CANUTILS_H__

#include "CAN.h"
#include "mbed.h"

#include "common.h"

#define CANsuppliedCLK 24000000

extern void can_disable(uint8_t chan);
extern void can_enable(uint8_t chan);
extern void can_configure(uint8_t chan, uint32_t baud, bool listen);
extern void can_add_filter(uint8_t chan, uint32_t id);
extern void can_reset_filters();
extern void can_use_filters(bool active);

extern void can_open();
extern void can_close();
extern void can_monitor();
extern void can_active();
extern uint8_t can_set_speed(uint32_t speed);
extern void show_can_message();
extern void show_T5can_message();
extern void show_T7can_message();
extern void show_T8can_message();
extern void silent_can_message();

extern bool can_send_timeout (uint32_t id, char *frame, uint8_t len, uint16_t timeout);
extern bool can_wait_timeout (uint32_t id, char *frame, uint8_t len, uint16_t timeout);

#endif
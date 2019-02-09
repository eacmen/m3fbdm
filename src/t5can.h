/*******************************************************************************

t5can.h - information and definitions needed for doing things with the T5 ECU
(c) 2010 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __T5CAN_H__
#define __T5CAN_H__

#include "mbed.h"
#include "CAN.h"

#include "common.h"
#include "strings.h"
#include "t5utils.h"
#include "srecutils.h"

#define T5SYMBOLS 'S'
#define T5VERSION 's'
#define T5WRITE 'W'

extern void t5_can();

void t5_can_show_help();
bool t5_can_show_can_message();
bool t5_can_get_symbol_table();
bool t5_can_get_version();
bool t5_can_get_adaption_data();
bool t5_can_send_boot_loader_S19();
bool t5_can_send_boot_loader();
bool t5_can_get_checksum();
bool t5_can_bootloader_reset();
bool t5_can_get_start_and_chip_types(uint32_t* start);
bool t5_can_erase_flash();
bool t5_can_dump_flash(uint32_t start);
bool t5_can_send_flash_s19_update(uint32_t start);
bool t5_can_send_flash_bin_update(uint32_t start);
bool t5_can_get_last_address();

#endif
/*******************************************************************************

bdmcpu32.h
(c) 2010 by Sophie Dexter

A derivative work based on:
//-----------------------------------------------------------------------------
//    CAN/BDM adapter firmware
//    (C) Janis Silins, 2010
//    $id$
//-----------------------------------------------------------------------------

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __BDMCPU32_H__
#define __BDMCPU32_H__

#include "mbed.h"

#include "common.h"
#include "bdmtrionic.h"
//#include "BDM.h"


// MCU status macros
#ifndef IGNORE_VCC_PIN
#define IS_CONNECTED    (PIN_PWR)
//#define IS_CONNECTED    (bool)((LPC_GPIO1->FIOPIN) & (1 << 30))     // PIN_POWER is p19 p1.30
#else
#define IS_CONNECTED    true
#endif    // IGNORE_VCC_PIN

#define IN_BDM              (PIN_FREEZE)
// #define IN_BDM              (bool)((LPC_GPIO2->FIOPIN) & (1 << 0))      // FREEZE is p26 P2.0
#define IS_RUNNING          (PIN_RESET && !IN_BDM)
//#define IS_RUNNING          ((bool)((LPC_GPIO2->FIOPIN) & (1 << 3)) && !IN_BDM)          // PIN_RESET is P23 P2.3

// MCU management
uint8_t stop_chip();
uint8_t reset_chip();
uint8_t run_chip(const uint32_t* addr);
uint8_t restart_chip();
uint8_t step_chip();
uint8_t bkpt_low();
uint8_t bkpt_high();
uint8_t reset_low();
uint8_t reset_high();
uint8_t berr_low();
uint8_t berr_high();
uint8_t berr_input();

// BDM Clock speed
enum bdm_speed {
    SLOW,
    FAST,
    TURBO,
    NITROUS
};
void bdm_clk_mode(bdm_speed mode);

// memory
uint8_t memread_byte(uint8_t* result, const uint32_t* addr);
uint8_t memread_word(uint16_t* result, const uint32_t* addr);
uint8_t memread_long(uint32_t* result, const uint32_t* addr);
uint8_t memdump_byte(uint8_t* result);
uint8_t memdump_word(uint16_t* result);
uint8_t memdump_long(uint32_t* result);
uint8_t memwrite_byte(const uint32_t* addr, uint8_t value);
uint8_t memwrite_word(const uint32_t* addr, uint16_t value);
uint8_t memwrite_long(const uint32_t* addr, const uint32_t* value);
uint8_t memfill_byte(uint8_t value);
uint8_t memfill_word(uint16_t value);
uint8_t memfill_long(const uint32_t* value);

// memory split commands
// Setup a start of a sequence of BDM operations
// read commands
uint8_t memread_byte_cmd(const uint32_t* addr);
uint8_t memread_word_cmd(const uint32_t* addr);
uint8_t memread_long_cmd(const uint32_t* addr);
// write commands
uint8_t memwrite_byte_cmd(const uint32_t* addr);
uint8_t memwrite_word_cmd(const uint32_t* addr);
uint8_t memwrite_long_cmd(const uint32_t* addr);
// follow on commands
// dump bytes/words/longs
uint8_t memget_word(uint16_t* result);
uint8_t memget_long(uint32_t* result);
// read and write bytes
uint8_t memwrite_write_byte(const uint32_t* addr, const uint8_t value);
uint8_t memwrite_read_byte(const uint32_t* addr, const uint8_t value);
uint8_t memwrite_nop_byte(const uint32_t* addr, const uint8_t value);
uint8_t memread_read_byte(uint8_t* result, const uint32_t* addr);
uint8_t memread_write_byte(uint8_t* result, const uint32_t* addr);
uint8_t memread_nop_byte(uint8_t* result, const uint32_t* addr);
//
uint8_t memwrite_word_write_word(const uint32_t* addr, const uint16_t value1, const uint16_t value2);
uint8_t memwrite_word_read_word(uint16_t* result, const uint32_t* addr, const uint16_t value);


// registers
uint8_t sysreg_read(uint32_t* result, uint8_t reg);
uint8_t sysreg_write(uint8_t reg, const uint32_t* value);
uint8_t adreg_read(uint32_t* result, uint8_t reg);
uint8_t adreg_write(uint8_t reg, const uint32_t* value);

// bdm part commands
bool bdm_command (uint16_t cmd);
bool bdm_address (const uint32_t* addr);
bool bdm_get (uint32_t* result, uint8_t size, uint16_t next_cmd);
bool bdm_put (const uint32_t* value, uint8_t size);
bool bdm_ready (uint16_t next_cmd);

#endif    // __BDMCPU32_H__
//-----------------------------------------------------------------------------
//    EOF
//-----------------------------------------------------------------------------
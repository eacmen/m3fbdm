/*******************************************************************************

bdmtrionic.cpp
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

#ifndef __BDMTRIONIC_H__
#define __BDMTRIONIC_H__

#include "mbed.h"

#include "common.h"
#include "bdmcpu32.h"
#include "bdmdriver.h"

#define FILE_BUF_LENGTH      0x1000              ///< file buffer size
static char file_buffer[FILE_BUF_LENGTH];     ///< file buffer

// global variables
static bool verify_flash = 1;

// public functions
uint8_t dump_flash(const uint32_t* start_addr, const uint32_t* end_addr);
uint8_t erase_flash(const char* flash_type, const uint32_t* start_addr,
    const uint32_t* end_addr);
uint8_t write_flash(const char* flash_type, const uint32_t* start_addr);

uint8_t prep_t5_do(void);
uint8_t prep_t8_do(void);
uint8_t dump_trionic(void);
uint8_t flash_trionic(void);

#endif
//-----------------------------------------------------------------------------
//    EOF
//-----------------------------------------------------------------------------

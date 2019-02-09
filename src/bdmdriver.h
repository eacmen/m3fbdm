/*******************************************************************************

bdmdriver.cpp
(c) 2014 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __BDMDRIVER_H__
#define __BDMDRIVER_H__

#include "mbed.h"

#include "common.h"
#include "bdmcpu32.h"

// public functions
bool bdmLoadMemory(uint8_t dataArray[], uint32_t loadAddress, uint32_t dataArraySize);
bool bdmRunDriver(uint32_t addr, uint32_t maxtime);
uint8_t bdmProcessSyscall(void);

enum bdmSyscall {
    QUIT = 0,
    PUTS = 1,
    PUTCHAR = 2,
    GETS = 3,
    GETCHAR = 4,
    GETSTAT = 5,
    FOPEN = 6,
    FCLOSE = 7,
    FREAD = 8,
    FWRITE = 9,
    FTELL = 10,
    FSEEK = 11,
    FGETS = 12,
    FPUTS = 13,
    EVAL = 14,
    FREADSREC = 15
};

enum bdmSyscallResult {
    DONE,
    CONTINUE,
    ERROR    
    };

#endif    // __BDMDRIVER_H__
//-----------------------------------------------------------------------------
//    EOF
//-----------------------------------------------------------------------------
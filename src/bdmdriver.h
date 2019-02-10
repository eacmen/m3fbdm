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
    BDM_QUIT = 0,
    BDM_PUTS = 1,
    BDM_PUTCHAR = 2,
    BDM_GETS = 3,
    BDM_GETCHAR = 4,
    BDM_GETSTAT = 5,
    BDM_FOPEN = 6,
    BDM_FCLOSE = 7,
    BDM_FREAD = 8,
    BDM_FWRITE = 9,
    BDM_FTELL = 10,
    BDM_FSEEK = 11,
    BDM_FGETS = 12,
    BDM_FPUTS = 13,
    BDM_EVAL = 14,
    BDM_FREADSREC = 15
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
/*******************************************************************************

srecutils.h
(c) 2010 by Sophie Dexter

Functions for manipulating Motorala S-records

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "srecutils.h"

// SRecGetByte
//
// Returns an int which is a single byte made up from two ascii characters read from an S-record file
//
// inputs:    a file pointer for the S-record file
// return:    an integer which is the byte in hex format

int SRecGetByte(FILE *fp) {
    int c = 0;
    int retbyte = 0;

    for(int i=0; i<2; i++) {
        if ((c = fgetc(fp)) == EOF) return -1;
        c -= (c > '9') ? ('A' - 10) : '0';
        retbyte = (retbyte << 4) + c;
    }
    return retbyte;
}

// SRecGetAddress
//
// Returns an int which is the address part of the S-record line
// The S-record type 1/2/3 or 9/8/7 determines if there are 2, 3 or 4 bytes in the address
//
// inputs:    an integer which is the number of bytes that make up the address; 2, 3 or 4 bytes
//            a file pointer for the S-record file
// return:    an integer which is the load address for the S-record

int SRecGetAddress(int size, FILE *fp) {
    int address = 0;
    for (int i = 0; i<size; i++)
    {
        address <<= 8;
        address |= SRecGetByte (fp);
    }
    return address;
}
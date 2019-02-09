/*******************************************************************************

srecutils.h - information and definitions needed for working with S-Records
(c) 2010 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "mbed.h"

#include "common.h"

extern int SRecGetByte(FILE *fp);
extern int SRecGetAddress(int size, FILE *fp);

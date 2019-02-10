/*******************************************************************************

strings.h - information and definitions needed for working with strings of
characters (ascii 'char') 
(c) 2010 by Sophie Dexter

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "common.h"

extern char *StrCpy(char *s1, char *s2);
extern int StrLen(char *s);
extern bool StrCmp(char *s1, char *s2);
extern char ToUpper(char c);
extern char ToLower(char c);
extern char *aToh(char *s);
extern char *StrAddc (char *s, const char c);
extern uint8_t ascii2nibble(char str);
extern bool ascii2int(uint32_t* val, const char* str, uint8_t length);
extern int isxdigit ( int ch );

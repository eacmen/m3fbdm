/*******************************************************************************

strings.cpp 
(c) 2010 by Sophie Dexter

This C++ module provides functions for working with 'strings' of ascii 'char's

C++ should have functions like these, but it doesn't seem to so these are my
own ones. They are very simple and do not check for any errors e.g. StrCpy
does not check that s1 is big enough to store all of s2.

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "strings.h"

// copies a string, s2, (array of chars) to another string, s1.
char *StrCpy(char *s1, char *s2) {
//    while (*s1++ = *s2++);
    while (*s2)
        *s1++ = *s2++;
    return s1;
}

// returns the number of chars in a string
int StrLen(char *s) {
    int x = 0;
    while (*s++)
        x++;
    return (x);
}

// checks s1 to see if it the same as s2
// returns TRUE if there is a match
// WARNING actually only checks that s1 starts with s2!
bool StrCmp(char *s1, char *s2) {
//    while (*s2 != '\0') {
    while (*s2) {
        if (*s1++ != *s2++) {
            return FALSE;
        }
    }
    return TRUE;
}

// Converts lower case ascii letters a-z to upper case
// leaves other characters unchanged
char ToUpper(char c) {
    if (c >= 'a' && c <= 'z')
        c -= 32;
    return c;
}

// Converts upper case ascii letters A-Z to lower case
// leaves other characters unchanged
char ToLower(char c) {
    if (c >= 'A' && c <= 'Z')
        c += 32;
    return c;
}

// Converts ASCII numbers 0-9 and letters a-f (and A-F) to hex values 0x00-0x0F
// leaves other characters unchanged
// ASCII '0' is worth 0x30 so subtract 0x30 if ascii character is a number
// Lower case ASCII letter 'a' is worth 0x61, but we want this to be worth 0x0A
// Subtract 0x57 (0x61 + 0x0A = 0x57) from lower case letters
// Upper case ASCII letter 'A' is worth 0x41, but we want this to be worth 0x0A
// Subtract 0x37 (0x41 + 0x0A = 0x37) from upper case letters

char *aToh(char *s) {
    while (*s) {
        if ((*s >= '0') && (*s <='9'))
            *s -= '0';
        else if ((*s >= 'a') && (*s <='f'))
            *s -= ('a' - 0x0A);
        else if ((*s >= 'A') && (*s <='F'))
            *s -= ('A' - 0x0A);
        s++;
    }
    return s;
}

// StrAddc    adds a single char to the end of a string

char *StrAddc (char *s, const char c) {
  char *s1 = s;

// Find the end of the string 's'
  while (*s)
    *s++;
// add the new character
  *s++ = c;
// put the end of string character at its new position
  *s = '\0';
  return s1;
}

//-----------------------------------------------------------------------------
/**
    Converts an ASCII character to low nibble of a byte.

    @param        rx_byte            character to convert
    
    @return                        resulting value            
*/
uint8_t ascii2nibble(char str)
{
    return str >= 'a' ? (str - 'a' + 10) & 0x0f :
        (str >= 'A' ? (str - 'A' + 10) & 0x0f :
        (str - '0') & 0x0f);
}

//-----------------------------------------------------------------------------
/**
    Converts an ASCII string to integer (checks string contents beforehand).

    @param        val                destination integer
    @param        str                pointer to source string
    @param        length            length of source string

    @return                        succ / fail
*/
bool ascii2int(uint32_t* val, const char* str, uint8_t length)
{
    // nothing to convert
    if (!str || length < 1)
    {
        *val = 0;
        return false;
    }

    // check string contents
    uint8_t shift;
    for (shift = 0; shift < length; ++shift)
    {
        if (!isxdigit(*(str + shift)))
        {
            // not a hex value
            *val = 0;
            return false;    
        } 
    }

    // convert string
    *val = ascii2nibble(*(str++));
    for (shift = 1; shift < length; ++shift)
    {
        *val <<= 4;
        *val += ascii2nibble(*(str++));
    }
    return true;
}

int isxdigit ( int ch )
{
    return (unsigned int)( ch         - '0') < 10u  ||
           (unsigned int)((ch | 0x20) - 'a') <  6u;
}

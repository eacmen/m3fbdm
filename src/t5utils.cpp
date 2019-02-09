/*******************************************************************************

t5utils.cpp
(c) 2010 by Sophie Dexter

This C++ module provides functions for communicating simple messages to and from
the T5 ECU

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "t5utils.h"

//
// T5Open enables the CAN chip and sets the CAN speed to 615 kbits.
//
// This function is a 'quick fix' for now.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//
bool T5Open() {
    can_open ();
    return TRUE;
}

//
// T5Close disables the CAN chip.
//
// This function is a 'quick fix' for now.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//
bool T5Close() {
    can_close ();
    return TRUE;
}

//
// T5WaitResponse
//
// Waits for a response message with the correct message id from the T5 ECU.
// The response is a single ascii character from the symboltable.
//
// Returns an error ('BELL' character) if:
//      a response is not received before the timeout
//      the message length is less than 8 bytes (all messages should be 8 bytes)
//      the message type is incorrect
//
// inputs:    none
// return:    a single char that is the response from the T5 ECU
//
char T5WaitResponse() {
    char T5RxMsg[8];
    if (!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) return '\a';
    if (T5RxMsg[0] != 0xC6) return '\a';
    return T5RxMsg[2];
}

//
//
// T5WaitResponsePrint
//
// Waits for a response message with the correct message id from the T5 ECU and displays the whole message:
//
// wiiildddddddddddddddd
//  iii - is the CAN message id
//     l - is the message length, should always be 8 for T5 messages
//      dd,dd,dd,dd,dd,dd,dd,dd are the CAN message data bytes
//
// Returns an error if:
//      a response is not received before the timeout
//      the message length is less than 8 bytes (all messages should be 8 bytes)
//      the message type is incorrect
//
// Inputs:    none
// return:    bool TRUE if a qualifying message was received
//                 FALSE if a message wasn't received in time, had the wrong id etc...
//
bool T5WaitResponsePrint() {
    char T5RxMsg[8];
    if (!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) return FALSE;
    printf("w%03x8", RESPID);
    for (char i=0; i<8; i++) {
        printf("%02x", T5RxMsg[i]);
    }
    printf("\n\r");
    return TRUE;
}

// T5GetSymbol
//
// Reads a single symbol name (in the symbol table) from the T5 ECU.
// T5GetSymbol sends ACK messages to the T5 ECU and stores the
// reply characters in a string until a '\n' character is received
// when the function returns with the pointer to that string.
//
// inputs:    a pointer to the string used to store the symbol name.
// return:    a pointer to the string used to store the symbol name.
//
char *T5GetSymbol(char *s) {
    do {
        if (T5Ack()) {
        }
        *s = T5WaitResponse();
    } while (*s++ != '\n');
    *s = '\0';
    return s;
}

// T5ReadCmnd
//
// Sends a 'read' type command to the T5 ECU. 'read' commands are used to read something from the T5 ECU.
//
// inputs:    a 'char' which is the type of 'read' requested (e.g. symbol table, version etc).
// return:    bool TRUE if the message was sent within the timeout period,
//                 FALSE if the message couldn't be sent.
//
bool T5ReadCmnd(char c) {
    char T5TxMsg[] = T5_RDCMND;
    T5TxMsg[1] = c;
    return (can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT));
}

//
// T5WriteCmnd is unfinished
//
bool T5WriteCmnd(unsigned int addr, char data) {
    return FALSE;
}



// t5_can_read_data
//
// Reads 6 bytes of the data from the SRAM or FLASH in the T5 ECU.
//
// There is something back-to-front about the way this works, but it does :-)
//
// inputs:    a pointer to an array of 6 bytes for storing the data read from the T5 ECU.
//            the address to read SRAM data from in the T5 ECU
// return:    a pointer to an array of 6 bytes for storing the data read from the T5 ECU.
//            bool TRUE if the message was sent within the timeout period,
//                 FALSE if the message couldn't be sent.
//
bool t5_can_read_data(char *data, uint32_t addr) {

    // send a can message to the T5 with the address we want to read from
    char T5TxMsg[] = T5RAMCMND;
    T5TxMsg[1] = ((uint8_t)(addr >> 24));         // high high byte of address
    T5TxMsg[2] = ((uint8_t)(addr >> 16));         // high low byte of address
    T5TxMsg[3] = ((uint8_t)(addr >> 8));          // low high byte of address
    T5TxMsg[4] = ((uint8_t)(addr));               // low low byte of address
    // if the message cannot be sent then there is a problem
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply with some data
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    if ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        return FALSE;
    for (int i=2; i<8; i++)
        data[7-i] = T5RxMsg[i];
    return TRUE;
}

// T5Ack
//
// Sends an 'ACK' message to the T5 ECU to prompt the T5 to send the next
// ascii character from the symboltable
//
// Returns an error if:
//      the 'ACK' message cannot be sent before the timeout
//
// inputs:    none
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.

bool T5Ack() {
    char T5TxMsg[] = T5ACKCMND;
    return (can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT));
}

//
// t5_can_send_boot_address
//
// Send an address where 'bootloader' or FLASH CAN messages are uploaded to.
// The number of bytes (up to 0x7F) that will be sent in the following CAN messages is also sent.
//
// inputs:    an unsigned int, the address where messages should be sent.
//            an int, the number of bytes (up to 0x7F) that will be sent in the following CAN messages.
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.
//
bool t5_can_send_boot_address(uint32_t addr, uint8_t len) {
    // send a can message to the T5 with the address and number of bytes we are going to send
    char T5TxMsg[] = T5_BTCMND;
    T5TxMsg[1] = ((uint8_t)(addr >> 24));         // high high byte of address
    T5TxMsg[2] = ((uint8_t)(addr >> 16));         // high low byte of address
    T5TxMsg[3] = ((uint8_t)(addr >> 8));          // low high byte of address
    T5TxMsg[4] = ((uint8_t)(addr));               // low low byte of address
    T5TxMsg[5] = (len);                           // number of bytes to upload (sent in following messages)
    // if the message cannot be sent then there is a problem
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    return ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        ? FALSE : TRUE;
}

//
// t5_can_send_boot_frame
//
// Upload 'bootloader' or new FLASH byts to T5 ECU in CAN messages
//
// The CAN messages are made up of 1 byte which is added to the address given in the T5SendBootAddress meaassge
// followed by 7 bytes of data.
// The first byte normally starts at 0x00 and increases by 7 in following messages until another T5SendBootAddress message is sent e.g.
// 00,dd,dd,dd,dd,dd,dd,dd,dd 'dd' bytes are the data bytes, 7 in each CAN message except the last one which may have less.
// 07,dd,dd,dd,dd,dd,dd,dd,dd
// 0E,dd,dd,dd,dd,dd,dd,dd,dd (0x0E is 14)
// 15,dd,dd,dd,dd,dd,dd,dd,dd (0x15 is 21)
// etc.
//
// inputs:    a pointer to an array of 8 bytes to be sent to the T5 ECU SRAM or FLASH.
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.
//
bool t5_can_send_boot_frame (char *T5TxMsg) {
    // send a can message to the T5 with the bytes we are sending
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    return ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        ? FALSE : TRUE;
}

//
// T5StartBootLoader
//
// Send the jump to address to the T5 ECU for starting the 'bootloader'.
// The jump address must be somewhere in RAM (addresses 0x0000 to 0x8000).
// (The start address comes from the S7/8/9 line in the S19 file.)
//
// inputs:    an unsigned int, the address to jump to start of 'bootloader'.
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.
//
bool T5StartBootLoader (uint32_t addr) {
    char T5TxMsg[] = T5JMPCMND;
    T5TxMsg[3] = ((uint8_t)(addr >> 8));          // low byte of address
    T5TxMsg[4] = ((uint8_t)(addr));               // high byte of address
    return (can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT));
}

//
// t5_boot_checksum_command
//
// Send the checksum command to Bootloader
//
// inputs:    none
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.
//
bool t5_boot_checksum_command(uint32_t* result) {
    *result = 0;
    // send a can message to the T5 requesting that it calculates the BIN file checksum
    char T5TxMsg[] = T5SUMCMND;
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    if ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5CHECKSUMTIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        return FALSE;
// get the checksum
    for (uint8_t i=0; i<4; i++) {
        *result <<= 8;
        *result |= T5RxMsg[i+2];
    }
    return TRUE;
}

//
// t5_boot_reset_command
//
// Send 'exit and restart T5' command to the Bootloader
//
// inputs:    none
// return:    bool TRUE if message was sent OK,
//                 FALSE if message could not be sent.
//
bool t5_boot_reset_command() {
    // send a can message to the T5 telling it to reset the ECU
    char T5TxMsg[] = T5RSTCMND;
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    return ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        ? FALSE : TRUE;
//    if ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
//        return FALSE;
    // wait for the T5 to reset
    // if a message is not received, has the wrong type or indicates an error then there is a problem
//    return (!can_wait_timeout(RSETID, T5RxMsg, 8, T5MESSAGETIMEOUT))
//        ? FALSE : TRUE;
}

//-----------------------------------------------------------------------------
/**
    t5_boot_get_flash_type
    Send 'get FLASH chip types' command to the Bootloader

    @param      result      start address of T5.x FLASH
                make        FLASH chip manufacturer code
                type        FLASH chip type code

    @return     bool        TRUE if message was sent OK,
                            FALSE if message could not be sent.
*/
bool t5_boot_get_flash_type(uint32_t* result, uint8_t* make, uint8_t* type) {
    *result = 0;
    *make = 0;
    *type = 0;
    // send a can message to the T5 requesting that it tells us the type of FLASH chips
    char T5TxMsg[] = T5TYPCMND;
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    if ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        return FALSE;
    // get the start address
    for (uint8_t i=0; i<4; i++) {
        *result <<= 8;
        *result |= T5RxMsg[i+2];
    }
    // get the manufacture code
    *make = T5RxMsg[6];
    // get the FLASH type code
    *type = T5RxMsg[7];
    return TRUE;
}

//-----------------------------------------------------------------------------
/**
    t5_boot_erase_command
    Send 'erase FLASH chip types' command to the Bootloader

    @param      none

    @return     bool        TRUE if message was sent OK,
                            FALSE if message could not be sent.
*/
bool t5_boot_erase_command() {
    // send a can message to the T5 telling it to erase the FLASH chips
    char T5TxMsg[] = T5ERACMND;
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    return ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5ERASETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        ? FALSE : TRUE;
}

//-----------------------------------------------------------------------------
/**
    t5_boot_c3_command
    Send 'C3 - Get last address' command to the Bootloader

    @param      result      last address of T5 FLASH
                mode        might indicate if bootloader is loaded

    @return     bool        TRUE if message was sent OK,
                            FALSE if message could not be sent.
*/
bool t5_boot_c3_command(uint32_t* result, uint16_t* mode) {
    *result = 0;
    *mode = 0;
    // send a can message to the T5 requesting that it tells us the last address of FLASH
    char T5TxMsg[] = T5EOFCMND;
    if (!can_send_timeout (CMNDID, T5TxMsg, 8, T5MESSAGETIMEOUT))
        return FALSE;
    // wait for the T5 to reply
    char T5RxMsg[8];
    // if a message is not received, has the wrong type or indicates an error then there is a problem
    if ((!can_wait_timeout(RESPID, T5RxMsg, 8, T5MESSAGETIMEOUT)) || (T5RxMsg[0] != T5TxMsg[0]) || (T5RxMsg[1] != 0x00))
        return FALSE;
// get the checksum
    for (uint8_t i=0; i<4; i++) {
        *result <<= 8;
        *result |= T5RxMsg[i+2];
    }
    for (uint8_t i=0; i<2; i++) {
        *mode <<= 8;
        *mode |= T5RxMsg[i+6];
    }
    return TRUE;
}
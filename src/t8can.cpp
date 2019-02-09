/*******************************************************************************

trionic8.cpp - CAN Bus functions for Just4Trionic by Just4pLeisure
(c) 2011, 2012 by Sophie Dexter

This C++ module provides functions for reading and writing the FLASH chips and
SRAM in Trionic8 ECUs. (Writing the adaption data back to SRAM not done yet).

Some functions need an additional 'bootloader' program to be sent to the T8 ECU
before they can be used. These functions are: Identifying the T5 ECU type and
FLASH chips, dumping the FLASH chips, erasing the FLASH chips, writing to the
FLASH chips and calculating the FLASH chips' checksum.

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "t8can.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer

//static uint32_t cmd_addr;                   ///< address (optional)
//static uint32_t cmd_value;                  ///< value    (optional)
//static uint32_t cmd_result;                 ///< result

//static uint32_t flash_start = 0;

// private functions
uint8_t execute_t8_cmd();
void t8_can_show_help();
void t8_can_show_full_help();

void t8_can()
{
    // Start the CAN bus system
    // Note that at the moment this is only for T8 ECUs at 500 kbits
    t8_can_show_help();

    char data[8];
    printf("Trying to listen to  CAN P-Bus (500 kBit/s)...\r\n");
    can_configure(2, 500000, 1);
    if (can_wait_timeout(T8ANYMSG, data, 8, T8MESSAGETIMEOUT)) {
        printf("Connected to Saab P-Bus\r\n");
        printf("Switching to P-Bus active mode\r\n");
        can_configure(2, 500000, 0);
    } else {
        printf("I did not receive any P-Bus messages\r\n");
        printf("Switching to P-Bus active mode\r\n");
        can_configure(2, 500000, 0);
        if (can_wait_timeout(T8ANYMSG, data, 8, T8CONNECTTIMEOUT)) {
            printf("Connected to Saab P-Bus\r\n");
            //can_active();
        } else {
            printf("FAILED to connect!\r\n");
            led4 = 1;
        }
    }

    can_reset_filters();
    can_add_filter(2, 0x645);         //645h - CIM
    can_add_filter(2, 0x7E0);         //7E0h -
    can_add_filter(2, 0x7E8);         //7E8h -
    can_add_filter(2, 0x311);         //311h -
    can_add_filter(2, 0x5E8);         //5E8h -
    //can_add_filter(2, 0x101);         //101h -


// main loop
    *cmd_buffer = '\0';
    char ret;
    char rx_char;
    while (true) {
        // read chars from USB
        // send received messages to the pc over USB connection
        // This function displays any CAN messages that are 'missed' by the other functions
        // Can messages might be 'missed' because they are received after a 'timeout' period
        // or because they weren't expected, e.g. if the T5 ECU resets for some reason
//        t7_show_can_message();
        silent_can_message();
        if (pc.readable()) {
            // turn Error LED off for next command
            led4 = 0;
            rx_char = pc.getc();
            switch (rx_char) {
                    // 'ESC' key to go back to mbed Just4Trionic 'home' menu
                case '\e':
                    can_close();
                    return;
                    // end-of-command reached
                case TERM_OK :
                    // execute command and return flag via USB
                    timer.reset();
                    timer.start();
                    ret = execute_t8_cmd();
                    pc.putc(ret);
                    printf("Completed in %.3f seconds.\r\n", timer.read());
                    // reset command buffer
                    *cmd_buffer = '\0';
                    // light up LED
                    //                    ret == TERM_OK ? led_on(LED_ACT) : led_on(LED_ERR);
                    ret == TERM_OK ? led3 = 1 : led4 = 1;
                    break;
                    // another command char
                default:
                    // store in buffer if space permits
                    if (StrLen(cmd_buffer) < CMD_BUF_LENGTH - 1) {
                        StrAddc(cmd_buffer, rx_char);
                    }
                    break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/**
    Executes a command and returns result flag (does not transmit the flag
    itself).

    @return                    command flag (success / failure)
*/
uint8_t execute_t8_cmd()
{

    char data[8];
//    uint8_t cmd_length = strlen(cmd_buffer);
    // command groups
    switch (*cmd_buffer) {
//            CHECK_ARGLENGTH(0);
            // Get the Symbol Table
        case 'i' :
            if (t8_initialise()) {
                printf("Trionic 8 Connection OK\r\n");
                return TERM_OK;
            } else {
                printf("Trionic 8 Connection Failed\r\n");
                return TERM_ERR;
            }
//            return t7_initialise()
//                   ? TERM_OK : TERM_ERR;
        case 'a' :
        case 'A' :
            if (t8_authenticate(T8REQID, T8RESPID, 0x01)) {
                printf("Security Key Accepted\r\n");
                return TERM_OK;
            } else {
                printf("Security Key Failed\r\n");
                return TERM_ERR;
            }
//            return t7_authenticate()
//                   ? TERM_OK : TERM_ERR;

// DUMP the T8 ECU BIN file stored in the FLASH chips
        case 'D':
        case 'd':
            return t8_dump()
                   ? TERM_OK : TERM_ERR;
// Send a FLASH update file to the T8 ECU
        case 'F':
        case 'f':
            return t8_flash()
                   ? TERM_OK : TERM_ERR;
// Erase the FLASH chips
        case 'r':
        case 'R':
            return t8_recover()
                   ? TERM_OK : TERM_ERR;
// Try to connect to CAN I-BUS
        case 'I' :
            printf("Trying to open CAN I-Bus (47619 Bit/s)...\r\n");
            can_close();
            //can_monitor();
            can_set_speed(47619);
            can_open();
            if (can_wait_timeout(T8ANYMSG, data, 8, T8CONNECTTIMEOUT)) {
                printf("Connected to Saab I-Bus\r\n");
                //can_active();
                return TERM_OK;
            } else {
                printf("I did not receive any I-Bus messages\r\n");
                printf("FAILED to connect!\r\n");
                return TERM_ERR;
            }
// Try to connect to CAN P-BUS
        case 'P' :
            printf("Trying to open CAN P-Bus (500 kBit/s)...\r\n");
            can_close();
            //can_monitor();
            can_set_speed(500000);
            can_open();
            if (can_wait_timeout(T8ANYMSG, data, 8, T8CONNECTTIMEOUT)) {
                printf("Connected to Saab P-Bus\r\n");
                //can_active();
                return TERM_OK;
            } else {
                printf("I did not receive any P-Bus messages\r\n");
                printf("FAILED to connect!\r\n");
                return TERM_ERR;
            }
// Show the VIN code
        case 'v':
            return t8_show_VIN()
                   ? TERM_OK : TERM_ERR;
        case 'V':
            return t8_write_VIN()
                   ? TERM_OK : TERM_ERR;
// Print help
        case 'h':
            t8_can_show_help();
            return TERM_OK;
        case 'H':
            t8_can_show_full_help();
            return TERM_OK;
        default:
            t8_can_show_help();
            break;
    }
// unknown command
    return TERM_ERR;
}

//
// Trionic7ShowHelp
//
// Displays a list of things that can be done with the T5 ECU.
//
// inputs:    none
// return:    none
//
void t8_can_show_help()
{
    printf("Trionic 8 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T8 ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T8\r\n");
    printf("R - Recover your T8 ECU FLASH with 'MODIFIED.BIN' file\r\n");
    printf("\r\n");
    printf("I - Try to open CAN I-Bus (47619 Bit/s)\r\n");
    printf("P - Try to open CAN P-Bus (500 kBit/s)\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("h  - Show this help menu\r\n");
    printf("\r\n");
    return;
}
//
// t7_can_show_full_help
//
// Displays a complete list of things that can be done with the T5 ECU.
//
// inputs:    none
// return:    none
//
void t8_can_show_full_help()
{
    printf("Trionic 8 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T8 ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T8\r\n");
    printf("R - Recover your T8 ECU FLASH with 'MODIFIED.BIN' file\r\n");
    printf("\r\n");
    printf("I - Try to open CAN I-Bus (47619 Bit/s)\r\n");
    printf("P - Try to open CAN P-Bus (500 kBit/s)\r\n");
    printf("\r\n");
    printf("\r\n");
    printf("i - Send initialisation message to T8\r\n");
    printf("a - Send Authentication key to T8\r\n");
    printf("d - Dump T8 Bin file 'ORIGINAL.BIN'\r\n");
    printf("e - Erase the FLASH in the T8 ECU\r\n");
    printf("f - FLASH the file 'MODIFIED.BIN' to the T8 ECU\r\n");
    printf("v - Display the current VIN code stored in the T8 ECU\r\n");
    printf("V - FLASH the file 'MODIFIED.BIN' to the T8 ECU\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("H  - Show this help menu\r\n");
    printf("\r\n");
    return;
}



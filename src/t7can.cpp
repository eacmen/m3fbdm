/*******************************************************************************

trionic7.cpp - CAN Bus functions for Just4Trionic by Just4pLeisure
(c) 2011, 2012 by Sophie Dexter

This C++ module provides functions for reading and writing the FLASH chips and
SRAM in Trionic7 ECUs. (Writing the adaption data back to SRAM not done yet).

Some functions need an additional 'bootloader' program to be sent to the T5 ECU
before they can be used. These functions are: Identifying the T5 ECU type and
FLASH chips, dumping the FLASH chips, erasing the FLASH chips, writing to the
FLASH chips and calculating the FLASH chips' checksum.

My version of the bootloader, BOOTY.S19, includes some features not in other
bootloaders; identifying the ECU and FLASH chip types, a 'safer' way of dumping
the FLASH chips and the ability to program AMD 29F010 type FLASH chips

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "t7can.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer

//static uint32_t cmd_addr;                   ///< address (optional)
//static uint32_t cmd_value;                  ///< value    (optional)
//static uint32_t cmd_result;                 ///< result

//static uint32_t flash_start = 0;

// private functions
uint8_t execute_t7_cmd();
void t7_can_show_help();
void t7_can_show_full_help();

// private variables
bool ibus = false;
bool pbus = false;

void t7_can()
{
    // Start the CAN bus system
    // Note that at the moment this is only for T7 ECUs at 500 kbits
    t7_can_show_help();

    char data[8];
    ibus = false;
    pbus = false;
    printf("Trying to listen to CAN I-Bus (47619 Bit/s)...\r\n");
    can_configure(2, 47619, 1);
    if (can_wait_timeout(T7ANYMSG, data, 8, T7MESSAGETIMEOUT)) {
        printf("Connected to Saab I-Bus\r\n");
        printf("Switching to I-Bus active mode\r\n");
        can_configure(2, 47619, 0);
        ibus = true;
    } else {
        printf("I did not receive any I-Bus messages\r\n");
        printf("Trying to listen to  CAN P-Bus (500 kBit/s)...\r\n");
        can_configure(2, 500000, 1);
        if (can_wait_timeout(T7ANYMSG, data, 8, T7MESSAGETIMEOUT)) {
            printf("Connected to Saab P-Bus\r\n");
            printf("Switching to P-Bus active mode\r\n");
            can_configure(2, 500000, 0);
            pbus = true;
        } else {
            printf("I did not receive any P-Bus messages\r\n");
            printf("Switching to P-Bus active mode\r\n");
            can_configure(2, 500000, 0);
            if (can_wait_timeout(T7ANYMSG, data, 8, T7CONNECTTIMEOUT)) {
                printf("Connected to Saab P-Bus\r\n");
                pbus = true;
                //can_active();
            } else {
                printf("FAILED to connect!\r\n");
                led4 = 1;
            }
        }
    }

//    if (t7_initialise())
//        printf("Connected to Saab I-Bus\r\n");
//    else {
//        printf("Opening CAN channel to Saab P-Bus (500 kBit/s)...\r\n");
//        can_set_speed(500000);
//        if (t7_initialise())
//            printf("Connected to Saab P-Bus\r\n");
//        else {
//            printf("FAILED to connect!\r\n");
//            led4 = 1;
//        }
//    }

//    if (t7_initialise())
//        printf("Trionic 7 Connection OK\r\n");
//    else
//        printf("Trionic 7 Connection Failed\r\n");
//    t7_authenticate();
//    if (t7_authenticate())
//        printf("Security Key Accepted\r\n");
//    else
//        printf("Security Key Failed\r\n");

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
                    ret = execute_t7_cmd();
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
uint8_t execute_t7_cmd()
{

//    uint8_t cmd_length = strlen(cmd_buffer);
    // command groups
    switch (*cmd_buffer) {
//            CHECK_ARGLENGTH(0);
        // Initialise a T7 CAN session
        case 'i' :
            if (t7_initialise()) {
                printf("SUCCESS! Trionic 7 CAN Connection OK.\r\n");
                return TERM_OK;
            } else {
                printf("FAILURE! No CAN connection to Trionic 7.\\r\n");
                return TERM_ERR;
            }
        // Get security clearance for T7 CAN session
        case 'a' :
            if (t7_authenticate()) {
                printf("SUCCESS! Security Key Accepted.\r\n");
                return TERM_OK;
            } else {
                printf("FAILURE! Unable to obtain a Security Key.\\r\n");
                return TERM_ERR;
            }
        // Erase the FLASH chips
        case 'e':
            return t7_erase()
                   ? TERM_OK : TERM_ERR;
        // DUMP the T5 ECU BIN file stored in the FLASH chips
        case 'D':
            if (!t7_authenticate()) {
                if (!t7_initialise()) {
                    printf("FAILURE! No CAN connection to Trionic 7.\r\n");
                    return TERM_ERR;
                }
                if (!t7_authenticate()) {
                    printf("FAILURE! Unable to obtain a Security Key.\r\n");
                    return TERM_ERR;
                }
            }
        case 'd':
            return t7_dump(pbus)
                   ? TERM_OK : TERM_ERR;
// Send a FLASH update file to the T5 ECU
        case 'F': {
            FILE *fp = t7_file_open("/local/modified.bin");    // Open "modified.bin" on the local file system
            if (!fp) {
                printf("FAILURE! Unable to find the BIN file \"MODIFIED.BIN\"\r\n");
                return TERM_ERR;
            }
            if (!t7_authenticate()) {
                if (!t7_initialise()) {
                    printf("FAILURE! No CAN connection to Trionic 7.\r\n");
                    fclose(fp);
                    return TERM_ERR;
                }
                if (!t7_authenticate()) {
                    printf("FAILURE! Unable to obtain a Security Key.\r\n");
                    fclose(fp);
                    return TERM_ERR;
                }
            }
            if (!t7_erase()) {
                printf("FAILURE: Unable to Erase FLASH!\r\n");
                fclose(fp);
                return TERM_ERR;
            }
            bool result = t7_flash(fp, pbus);
            fclose(fp);
            return result ? TERM_OK : TERM_ERR;
        }
        case 'f': {
            FILE *fp = t7_file_open("/local/modified.bin");    // Open "modified.bin" on the local file system
            if (!fp) {
                printf("FAILURE! Unable to find the BIN file \"MODIFIED.BIN\"\r\n");
                return TERM_ERR;
            }
            bool result = t7_flash(fp, pbus);
            fclose(fp);
            return result ? TERM_OK : TERM_ERR;
        }
// Recovery FLASHes the entire BIN file and using 'safe' but slow 4 byte at a time transfers
        case 'R': {
            FILE *fp = t7_file_open("/local/modified.bin");    // Open "modified.bin" on the local file system
            if (!fp) {
                printf("FAILURE! Unable to find the BIN file \"MODIFIED.BIN\"\r\n");
                return TERM_ERR;
            }
            if (!t7_authenticate()) {
                if (!t7_initialise()) {
                    printf("FAILURE! No CAN connection to Trionic 7.\r\n");
                    fclose(fp);
                    return TERM_ERR;
                }
                if (!t7_authenticate()) {
                    printf("FAILURE! Unable to obtain a Security Key.\r\n");
                    fclose(fp);
                    return TERM_ERR;
                }
            }
            if (!t7_erase()) {
                printf("FAILURE: Unable to Erase FLASH!\r\n");
                fclose(fp);
                return TERM_ERR;
            }
            bool result = t7_recover(fp);
            fclose(fp);
            return result ? TERM_OK : TERM_ERR;
        }
// Try to connect to CAN I-BUS
        case 'I' : {
            char data[8];
            printf("Trying to open CAN I-Bus (47619 Bit/s)...\r\n");
            ibus = true;
            pbus = false;
            can_close();
            //can_monitor();
            can_set_speed(47619);
            can_open();
            if (can_wait_timeout(T7ANYMSG, data, 8, T7CONNECTTIMEOUT)) {
                printf("SUCCESS! Connected to Saab I-Bus.\r\n");
                //can_active();
                return TERM_OK;
            } else {
                printf("I did not receive any I-Bus messages\r\n");
                printf("FAILURE! Unable to connect Saab I-Bus.\r\n");
                return TERM_ERR;
            }
        }
// Try to connect to CAN P-BUS
        case 'P' : {
            char data[8];
            printf("Trying to open CAN P-Bus (500 kBit/s)...\r\n");
            ibus = false;
            pbus = true;
            can_close();
            //can_monitor();
            can_set_speed(500000);
            can_open();
            if (can_wait_timeout(T7ANYMSG, data, 8, T7CONNECTTIMEOUT)) {
                printf("SUCCESS! Connected to Saab P-Bus.\r\n");
                //can_active();
                return TERM_OK;
            } else {
                printf("I did not receive any P-Bus messages\r\n");
                printf("FAILURE! Unable to connect Saab P-Bus.\r\n");
                return TERM_ERR;
            }
        }
// Print help
        case 'h':
            t7_can_show_help();
            return TERM_OK;
        case 'H':
            t7_can_show_full_help();
            return TERM_OK;
        default:
            t7_can_show_help();
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
void t7_can_show_help()
{
    printf("Trionic 7 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T7 ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T7\r\n");
    printf("R - Recovery FLASH T7 with update file 'MODIFIED.BIN'\r\n");
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
void t7_can_show_full_help()
{
    printf("Trionic 7 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T7 ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T7\r\n");
    printf("R - Recovery FLASH T7 with update file 'MODIFIED.BIN'\r\n");
    printf("\r\n");
    printf("I - Try to open CAN I-Bus (47619 Bit/s)\r\n");
    printf("P - Try to open CAN P-Bus (500 kBit/s)\r\n");
    printf("\r\n");
    printf("\r\n");
    printf("i - Send initialisation message to T7\r\n");
    printf("a - Send Authentication key to T7\r\n");
    printf("d - Dump T7 Bin file 'ORIGINAL.BIN'\r\n");
    printf("e - Erase the FLASH in the T7 ECU\r\n");
    printf("f - FLASH the file 'MODIFIED.BIN' to the T7 ECU\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("H  - Show this help menu\r\n");
    printf("\r\n");
    return;
}



/*******************************************************************************

trionic5.cpp - CAN Bus functions for Just4Trionic by Just4pLeisure
(c) 2010 by Sophie Dexter

This C++ module provides functions for reading and writing the FLASH chips and
SRAM in Trionic5 ECUs. (Writing the adaption data back to SRAM not done yet).

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

#include "t5can.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer

//static uint32_t cmd_addr;                   ///< address (optional)
//static uint32_t cmd_value;                  ///< value    (optional)
//static uint32_t cmd_result;                 ///< result

static uint32_t flash_start = 0;

// private functions
uint8_t execute_t5_cmd();
void t5_can_show_help();
void t5_can_show_full_help();

void t5_can()
{
    // Start the CAN bus system
    // Note that at the moment this is only for T5 ECUs at 615 kbits
    can_open();
    can_set_speed(615000);

    t5_can_show_help();

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
        t5_can_show_can_message();
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
                    ret = execute_t5_cmd();
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
uint8_t execute_t5_cmd()
{


//    uint8_t cmd_length = strlen(cmd_buffer);
    // command groups
    switch (*cmd_buffer) {
//            CHECK_ARGLENGTH(0);
            // Get the Symbol Table
        case 's':
            return t5_can_get_symbol_table()
                   ? TERM_OK : TERM_ERR;
        case 'S':
            return T5ReadCmnd(T5SYMBOLS)
                   ? TERM_OK : TERM_ERR;

            // Get the Trionic5 software version string
        case 'v':
            return t5_can_get_version()
                   ? TERM_OK : TERM_ERR;
        case 'V':
            return T5ReadCmnd(T5VERSION)
                   ? TERM_OK : TERM_ERR;

            // Read Adaption Data from RAM and write it to a file
        case 'r':
        case 'R':
            return t5_can_get_adaption_data()
                   ? TERM_OK : TERM_ERR;

            // CR - send CR type message
        case '\0':
            return T5ReadCmnd(CR)
                   ? TERM_OK : TERM_ERR;

            //  Get a single symbol from the Symbol Table
        case 'a':
            char symbol[40];
            T5GetSymbol(symbol);
            printf("%s",symbol);
            return TERM_OK;

            // Just send an 'ACK' message
        case 'A':
            return T5Ack()
                   ? TERM_OK : TERM_ERR;

            // Send a Bootloader file to the T5 ECU
        case 'b':
            return (t5_can_send_boot_loader() && can_set_speed(1000000))
                   ? TERM_OK : TERM_ERR;
        case 'B':
            return (t5_can_send_boot_loader_S19() && can_set_speed(1000000))
                   ? TERM_OK : TERM_ERR;

            // Get Checksum from ECU (Bootloader must be uploaded first)
        case 'c':
        case 'C':
            return t5_can_get_checksum()
                   ? TERM_OK : TERM_ERR;

            // Exit the BootLoader and restart the T5 ECU
        case 'q':
        case 'Q':
            return (t5_can_bootloader_reset() && can_set_speed(615000))
                   ? TERM_OK : TERM_ERR;

            // Erase the FLASH chips
        case 'e':
        case 'E':
            return t5_can_erase_flash()
                   ? TERM_OK : TERM_ERR;

            // Read back the FLASH chip types
        case 't':
        case 'T':
            return t5_can_get_start_and_chip_types(&flash_start)
                   ? TERM_OK : TERM_ERR;

            // DUMP the T5 ECU BIN file stored in the FLASH chips
        case 'd':
            // NOTE 'd' command Just4TESTING! only dumps T5.5 ECU
            return t5_can_dump_flash(T55FLASHSTART)
                   ? TERM_OK : TERM_ERR;
        case 'D':
//            if (!t5_can_send_boot_loader_S19())
            if (!t5_can_send_boot_loader())
                return TERM_ERR;
            can_set_speed(1000000);
            if (!t5_can_get_start_and_chip_types(&flash_start)) {
                t5_can_bootloader_reset();
                can_set_speed(615000);
                return TERM_ERR;
            }
            return (t5_can_dump_flash(flash_start) && t5_can_bootloader_reset() && can_set_speed(615000))
                   ? TERM_OK : TERM_ERR;

            // Send a FLASH update file to the T5 ECU
        case 'f':
            // NOTE 'f' command Just4TESTING! only FLASHes T5.5 ECU (with S19 type file)
            //return t5_can_send_flash_s19_update(T55FLASHSTART)
            return t5_can_send_flash_bin_update(T55FLASHSTART)
                   ? TERM_OK : TERM_ERR;
        case 'F':
//            if (!t5_can_send_boot_loader_S19())
            if (!t5_can_send_boot_loader())
                return TERM_ERR;
            can_set_speed(1000000);
            if (!t5_can_get_start_and_chip_types(&flash_start)) {
                t5_can_bootloader_reset();
                can_set_speed(615000);
                return TERM_ERR;
            }
            if (!t5_can_get_checksum())
                led4 = 1;
            if (!t5_can_erase_flash()) {
                t5_can_bootloader_reset();
                can_set_speed(615000);
                return TERM_ERR;
            }
            return (t5_can_send_flash_bin_update(flash_start) && t5_can_get_checksum() && t5_can_bootloader_reset() && can_set_speed(615000))
                   ? TERM_OK : TERM_ERR;

            // Send the C3 message - should get last used address 0x7FFFF
        case '3':
            return t5_can_get_last_address()
                   ? TERM_OK : TERM_ERR;

            // Print help
        case 'h':
            t5_can_show_help();
            return TERM_OK;
        case 'H':
            t5_can_show_full_help();
            return TERM_OK;
        default:
            t5_can_show_help();
            break;
    }
    // unknown command
    return TERM_ERR;
}

//
// Trionic5ShowHelp
//
// Displays a list of things that can be done with the T5 ECU.
//
// inputs:    none
// return:    none
//
void t5_can_show_help()
{
    printf("Trionic 5 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T5.x ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T5.x\r\n");
    printf("\r\n");
    printf("r - read SRAM and write it to ADAPTION.RAM file\r\n");
    printf("s - read Symbol Table and write it to SYMBOLS.TXT\r\n");
    printf("v - read T5 ECU software version, display it and write it to VERSION.TXT\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("h  - Show this help menu\r\n");
    printf("\r\n");
    return;
}
//
// t5_can_show_full_help
//
// Displays a complete list of things that can be done with the T5 ECU.
//
// inputs:    none
// return:    none
//
void t5_can_show_full_help()
{
    printf("Trionic 5 Command Menu\r\n");
    printf("======================\r\n");
    printf("D - DUMP the T5.x ECU FLASH to a file 'ORIGINAL.BIN'\r\n");
    printf("F - FLASH the update file 'MODIFIED.BIN' to the T5.x\r\n");
    printf("\r\n");
    printf("b - upload and start MyBooty.S19 bootloader\r\n");
    printf("c - get T5 ECU FLASH checksum (need to upload BOOTY.S19 before using this command)\r\n");
    printf("d - dump the T5 FLASH BIN file and write it to ORIGINAL.BIN\r\n");
    printf("e - erase the FLASH chips in the T5 ECU\r\n");
    printf("f - FLASH the update file MODIFIED.BIN to the T5 ECU\r\n");
    printf("r - read SRAM and write it to ADAPTION.RAM file\r\n");
    printf("s - read Symbol Table, display it and write it to SYMBOLS.TXT\r\n");
    printf("v - read T5 ECU software version, display it and write it to VERSION.TXT\r\n");
    printf("q - exit the bootloader and reset the T5 ECU\r\n");
    printf("t - read the FLASH chip type in the T5 ECU\r\n");
    printf("3 - read the last used FLASH address in the T5 ECU\r\n");
    printf("S - send 's' message (to get symbol table)\r\n");
    printf("V - send 'S' message (to get software version)\r\n");
    printf("'Enter' Key - send an CR message\r\n");
    printf("a - send an ACK\r\n");
    printf("A - read a single symbol from the symbol table\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("H  - Show this help menu\r\n");
    printf("\r\n");
    return;
}

//
// t5_can_show_can_message
//
// Displays a CAN message in the RX buffer if there is one.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
bool t5_can_show_can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        printf("w%03x%d", can_MsgRx.id, can_MsgRx.len);
        for (char i=0; i<can_MsgRx.len; i++) {
            printf("%02x", can_MsgRx.data[i]);
        }
        printf(" %c ", can_MsgRx.data[2]);
        printf("\r\n");
        return TRUE;
    }
    return FALSE;
}

//
// t5_can_get_symbol_table
//
// Gets the T5 ECU symbol table.
// The Symbol Table is saved to a file, symbols.txt, on the mbed 'local' file system 'disk'.
//
// inputs:    none
// return:    bool TRUE if there all went OK, FALSE if there was an error
//
bool t5_can_get_symbol_table()
{
    printf("Saving the symbol table file\r\n");
    FILE *fp = fopen("/local/symbols.txt", "w");  // Open "symbols.txt" on the local file system for writing
    if (!fp) {
        perror ("The following error occured");
        return FALSE;
    }
    char symbol[40];
    T5ReadCmnd(T5SYMBOLS);
    if (T5WaitResponse() != '>') {
        fclose(fp);
        return FALSE;
    }
    T5ReadCmnd(CR);
    if (T5WaitResponse() != '>') {
        fclose(fp);
        return FALSE;
    }
    do {
        T5GetSymbol(symbol);
//        printf("%s",symbol);
        if (fprintf(fp,"%s",symbol) < 0) {
            fclose (fp);
            printf ("ERROR Writing to the symbols.txt file!\r\n");
            return FALSE;
        };
    } while (!StrCmp(symbol,"END\r\n"));
    fclose(fp);
    return TRUE;
}

//
// t5_can_get_version
//
// Gets the T5 software version string.
// The software version is is sent to the PC and saved to a file, version.txt, on the mbed 'local' file system 'disk'.
//
// inputs:    none
// return:    bool TRUE if there all went OK, FALSE if there was an error
//
bool t5_can_get_version()
{
    FILE *fp = fopen("/local/version.txt", "w");  // Open "version.txt" on the local file system for writing
    if (!fp) {
        perror ("The following error occured");
        return FALSE;
    }
    char symbol[40];
    T5ReadCmnd(T5VERSION);
    if (T5WaitResponse() != '>') {
        fclose(fp);
        return FALSE;
    }
    T5ReadCmnd(CR);
    if (T5WaitResponse() != '>') {
        fclose(fp);
        return FALSE;
    }
    T5GetSymbol(symbol);
    printf("%s",symbol);
    if (fprintf(fp,"%s",symbol) < 0) {
        fclose (fp);
        printf ("ERROR Writing to the version.txt file!\r\n");
        return FALSE;
    };
    fclose(fp);
    printf("The version.txt file has been saved.\r\n");
    return TRUE;
}

//
// Trionic5GetAdaptionData
//
// Gets the adaption data from the T5's SRAM.
// The adaption data is stored in a hex file, adaption.RAM, on the mbed 'local' file system 'disk'.
//
// Reading the Adaption data from SRAM takes about 6.5 seconds.
//
// inputs:    none
// return:    bool TRUE if all went OK, FALSE if there was an error.
//
bool t5_can_get_adaption_data()
{
    printf("Saving the SRAM adaption data.\r\n");
    FILE *fp = fopen("/local/adaption.RAM", "w");    // Open "adaption.RAM" on the local file system for writing
    if (!fp) {
        printf("ERROR: Unable to open a file for the adaption data!\r\n");
        return FALSE;
    }
    unsigned int address = 5;                       // Mysterious reason for starting at 5 !!!
    char RAMdata[6];
    while (address < T5RAMSIZE) {
        if (!t5_can_read_data(RAMdata, address)) {
            fclose (fp);
            printf ("Error reading from the CAN bus.\r\n");
            return FALSE;
        }
        address += 6;
        if (fwrite(RAMdata, 1, 6, fp) != 6) {
            fclose (fp);
            printf ("Error writing to the SRAM adaption file.\r\n");
            return FALSE;
        }
    }
    // There are a few more bytes to get because because the SRAM file is not an exact multiple of 6 bytes!
    // the % (modulo) mathematics function tells us how many bytes there are still to get
    if (!t5_can_read_data(RAMdata, (T5RAMSIZE - 1))) {
        fclose (fp);
        printf ("Error reading from the CAN bus.\r\n");
        return FALSE;
    }
    if (fwrite((RAMdata + 6 - (T5RAMSIZE % 6)), 1, (T5RAMSIZE % 6), fp) != (T5RAMSIZE % 6)) {
        fclose (fp);
        printf ("Error writing to the SRAM adaption file.\r\n");
        return FALSE;
    }
    fclose(fp);
    return TRUE;
}

//
// t5_can_send_boot_loader
//
// Sends a 'bootloader' file, booty.s19 to the T5 ECU.
// The 'bootloader' is stored on the mbed 'local' file system 'disk' and must be in S19 format.
//
// The 'bootloader' is then able to dump or reFLASH the T5 ECU FLASH chips - this is the whole point of the exercise :-)
//
// Sending the 'bootloader' to the T5 ECU takes just over 1 second.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//                 FALSE if the 'bootloader' wasn't sent for some reason.
//
bool t5_can_send_boot_loader()
{
    uint32_t BootloaderSize = sizeof(T5BootLoader);
    uint32_t address = MYBOOTY_START; // Start and execute from address of T5Bootloader
    uint32_t count = 0; // progress count of bootloader bytes transferred
    char msg[8]; // Construct the bootloader frame for uploading

    printf("Starting the bootloader.\r\n");
    while (count < BootloaderSize) {
// send a bootloader address message
        if (!t5_can_send_boot_address((address+count), MYBOOTY_CHUNK)) return FALSE;
// send bootloader frames
// NOTE the last frame sent may have less than 7 real data bytes but 7 bytes are always sent. In this case the unnecessary bytes
// are repeated from the previous frame. This is OK because the T5 ECU knows how many bytes to expect (because the count of bytes
// is sent with the upload address) and ignores any extra bytes in the last frame.
        for (uint8_t i=0; i<MYBOOTY_CHUNK; i++) {
            msg[1+(i%7)] = T5BootLoader[count+i];
            if (i%7 == 0) msg[0]=i;     // set the index number
            if ((i%7 == 6) || (i == MYBOOTY_CHUNK-1 )) {
                if (!t5_can_send_boot_frame(msg)) return FALSE;
            }
        }
        count += MYBOOTY_CHUNK;
    }
// These two lines really shouldn't be necessary but for some reason the first start
// command is ignored and a short delay is required before repeating. Using only a
// delay, even a very long one, doesn't work.
//
// NOTE: This measure isn't required when uploading an external S19 bootloader file!
//
    T5StartBootLoader(address);
    wait_ms(1);
//
    return T5StartBootLoader(address);
}


//
// t5_can_send_boot_loader_S19
//
// Sends a 'bootloader' file, booty.s19 to the T5 ECU.
// The 'bootloader' is stored on the mbed 'local' file system 'disk' and must be in S19 format.
//
// The 'bootloader' is then able to dump or reFLASH the T5 ECU FLASH chips - this is the whole point of the exercise :-)
//
// Sending the 'bootloader' to the T5 ECU takes just over 1 second.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//                 FALSE if the 'bootloader' wasn't sent for some reason.
//
bool t5_can_send_boot_loader_S19()
{
    printf("Starting the bootloader.\r\n");
    FILE *fp = fopen("/local/MyBooty.S19", "r");    // Open "booty.s19" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the bootloader file MyBooty.S19\r\n");
        return FALSE;
    }
    int c = 0; // for some reason fgetc returns an int instead of a char
    uint8_t count = 0; // count of bytes in the S-record can be 0xFF = 255 in decimal
    uint8_t asize = 0; // 2,3 or 4 bytes in address
    uint32_t address = 0; // address to put S-record
    uint8_t checksum = 0; // checksum check at the end of each S-record line
    char msg[8]; // Construct the bootloader frame for uploading
    bool sent = FALSE;

    while (!sent) {
// get characters until we get an 'S' (throws away \n,\r characters and other junk)
        do c = fgetc (fp);
        while (c != 'S' && c != EOF);
//                if (c == EOF) return '\a';
        c = fgetc(fp);        // get the S-record type 1/2/3 5 7/8/9 (Return if EOF reached)
//                if ((c = fgetc(fp)) == EOF) return '\a';        // get the S-record type 1/2/3 5 7/8/9 (Return if EOF reached)
        switch (c) {
            case '0':
                break;                  // Skip over S0 header record

            case '1':
            case '2':
            case '3':
                asize = 1 + c - '0';    // 2, 3 or 4 bytes for address
                address = 0;
// get the number of bytes (in ascii format) in this S-record line
// there must be at least the address and the checksum so return with an error if less than this!
                if ((c = SRecGetByte(fp)) < (asize + 1)) break;
//                        if ((c = SRecGetByte(fp)) < 3) return '\a';
                count = c;
                checksum = c;
// get the address
                for (uint8_t i=0; i<asize; i++) {
                    c = SRecGetByte(fp);
                    checksum += c;
                    address <<= 8;
                    address |= c;
                    count--;
                }
// send a bootloader address message
////                printf("address %x count %x\r\n",address ,count-1 );
                if (!t5_can_send_boot_address(address, (count-1))) return FALSE;
// get and send the bootloader frames for this S-record
// NOTE the last frame sent may have less than 7 real data bytes but 7 bytes are always sent. In this case the unnecessary bytes
// are repeated from the previous frame. This is OK because the T5 ECU knows how many bytes to expect (because the count of bytes
// in the S-Record is sent with the upload address) and ignores any extra bytes in the last frame.
                for (uint8_t i=0; i<count-1; i++) {
                    c = SRecGetByte(fp);
                    checksum += c;
                    msg[1+(i%7)] = c;
                    if (i%7 == 0) msg[0]=i;     // set the index number
                    if ((i%7 == 6) || (i == count - 2)) {
////                        printf("Sending %2x %2x %2x %2x %2x %2x %2x %2x \r\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7] );
                        if (!t5_can_send_boot_frame(msg)) {
                            fclose(fp);
                            return FALSE;
                        }
                    }
                }
// get the checksum
                if ((checksum += SRecGetByte(fp)) != 0xFF) {
                    printf("Error in S-record, checksum = %2x\r\n", checksum);
                    fclose(fp);
                    return FALSE;
                }
                break;

            case '5':
                break;                  // Skip over S5 record types

            case '7':
            case '8':
            case '9':
                asize = 11 - (c - '0');  // 2, 3 or 4 bytes for address
                address = 0;
// get the number of bytes (in ascii format) in this S-record line there must be just the address and the checksum
// so return with an error if other than this!
                if ((c = SRecGetByte(fp)) != (asize + 1)) break;
//                if ((c = SRecGetByte(fp)) < 3) return '\a';
                checksum = c;
// get the address
                for (uint8_t i=0; i<asize; i++) {
                    c = SRecGetByte(fp);
                    checksum += c;
                    address <<= 8;
                    address |= c;
                }
// get the checksum
                if ((checksum += SRecGetByte(fp)) != 0xFF) {
                    fclose(fp);
                    printf("Error in S-record, checksum = %2x\r\n", checksum);
                    return FALSE;
                }
                T5StartBootLoader(address);
//                T5WaitResponsePrint();
                sent = TRUE;
                break;

// Some kind of invalid S-record type so break
            default:
                fclose(fp);
                printf("oops - didn't recognise that S-Record \r\n");
                return FALSE;
        }
    }
    fclose(fp);
    return TRUE;
}

//
// t5_can_get_checksum
//
// Calculates the checksum of the FLASH in the T5 ECU.
// The 'bootloader', booty.s19, must be loaded before this function can be used.
// The 'bootloader' actually calculates the checksum and compares it with the
// value stored in the 'header' region at the end of the FLASH.
//
// The bootloader sends a single CAN message with the result e.g.
// w00C8C800CAFEBABE0808
//  00C - T5 response messages have an CAN id of 00C
//     8 - All T5 messages have a message length of 8 bytes
//      C8 - This is the checksum message type
//        00 - 00 means OK, the checksum calculation matches the stored value which in this case is:
//          CAFEBABE - lol :-)
//
// w00C8C801FFFFFFFF0808
//        01 - 01 means calculated value doesn't matched the stored value
//          FFFFFFFF - in this case the stored value is FFFFFFFF - the chips might be erased
//
// Calculating the checksum takes a little under 2 seconds.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//
bool t5_can_get_checksum()
{
    uint32_t checksum = 0;
    if (!t5_boot_checksum_command(&checksum)) {
        printf("Error The ECU's checksum is wrong!\r\n");
        return FALSE;
    }
    printf("The FLASH checksum value is %#010x.\r\n", checksum);
    return TRUE;
}

//
// t5_can_bootloader_reset
//
// Exit the Bootloader and restart the T5 ECU
//
// inputs:    none
// outputs:    bool TRUE if all went OK.
//
bool t5_can_bootloader_reset()
{
    printf("Exiting the bootloader and restarting the T5 ECU.\r\n");
    if (!t5_boot_reset_command()) {
        printf("Error trying to reset the T5 ECU!\r\n");
        return FALSE;
    }
    return TRUE;
}

//
// t5_can_get_start_and_chip_types
//
// Gets the FLASH chip type fitted.
//
// NOTE the bootloader must be loaded in order to use this function.
//
// CAN messages from the T5 ECU with FLASH data look like this:
//
// C9,00,aa,aa,aa,aa,mm,dd,
//
// C9 lets us know its a FLASH id message
//
// aa,aa,aa,aa is the FLASH start address which we can use to work out if this is a T5.2 or T5.5 ECU
// 0x00020000 - T5.2
// 0x00040000 - T5.5
//
// mm = Manufacturer id. These can be:
// 0x89 - Intel
// 0x31 - CSI/CAT
// 0x01 - AMD
// 0x1F - Atmel
//
// dd = Device id. These can be:
// 0xB8 - Intel _or_ CSI 28F512 (Fiited by Saab in T5.2)
// 0xB4 - Intel _or_ CSI 28F010 (Fitted by Saab in T5.5)
// 0x25 - AMD 28F512 (Fiited by Saab in T5.2)
// 0xA7 - AMD 28F010 (Fitted by Saab in T5.5)
// 0x20 - AMD 29F010 (Some people have put these in their T5.5)
// 0x5D - Atmel 29C512 (Some people mave have put these in their T5.2)
// 0xD5 - Atmel 29C010 (Some people have put these in their T5.5)
//
// mm = 0xFF, dd == 0xF7 probably means that the programming voltage isn't right.
//
// Finding out which ECU type and FLASH chips are fitted takes under a second.
//
// inputs:    start     T5 ecu start address
// return:    bool      TRUE if all went OK.
//
bool t5_can_get_start_and_chip_types(uint32_t* start)
{
    *start = 0;
    uint8_t make = 0;
    uint8_t type = 0;
    if (!t5_boot_get_flash_type(start, &make, &type)) {
        printf("Error trying to find out the ECU's first address and FLASH chip type!\r\n");
        return FALSE;
    }
    printf("This is a T5.%s ECU with ", ((*start == T52FLASHSTART) ? "2" : "5"));
    switch (make) {
        case AMD:
            printf("AMD ");
            break;
        case CSI:
            printf("CSI ");
            break;
        case INTEL:
            printf("INTEL ");
            break;
        case ATMEL:
            printf("ATMEL ");
            break;
        case SST:
            printf("SST ");
            break;
        case ST:
            printf("ST ");
            break;
        case AMIC:
            printf("AMIC ");
            break;
        default:
            printf("\r\nUNKNOWN Manufacturer Id: %02x - Also check pin65 has enough volts!\r\n", make);
    }
    switch (type) {
        case AMD28F512:
        case INTEL28F512:
            printf("28F512 FLASH chips.\r\n");
            break;
        case ATMEL29C512:
            printf("29C512 FLASH chips.\r\n");
            break;
        case ATMEL29C010:
            printf("29C010 FLASH chips.\r\n");
            break;
        case AMD28F010:
        case INTEL28F010:
            printf("28F010 FLASH chips.\r\n");
            break;
        case AMD29F010:
        case SST39SF010:
//        case ST29F010:      // Same as AMD29F010
        case AMICA29010L:
            printf("29F010 FLASH chips.\r\n");
            break;
        default:
            printf("UNKNOWN Device Id: %02x - Also check pin65 has enough volts!\r\n", type);
            return FALSE;
    }
    return TRUE;
}

//
// t5_can_erase_flash
//
// Erases the FLASH Chips.
//
// NOTE the bootloader must be loaded in order to use this function.
//
// CAN messages from the T5 ECU with FLASH erase command look like this:
//
// C0,cc,08,08,08,08,08,08
//
// C0 tells us this is a response to the FLASH erase command.
//
// cc is a code that tells us what happened:
//      00 - FLASH was erased OK
//      01 - Could not erase FLASH chips to 0xFF
//      02 - Could not write 0x00 to 28F FLASH chips
//      03 - Unrecognised FLASH chip type (or maybe programming voltage isn't right)
//      04 - Intel chips found, but unrecognised type
//      05 - AMD chips found, but unrecognised type
//      06 - CSI/Catalyst chips found, but unrecognised type
//      07 - Atmel chips found - Atmel chips don't need to be erased
//
// Erasing 28F type FLASH chips takes around 22 seconds. 29F chips take around 4 seconds.
//
// inputs:    none
// return:    bool TRUE if all went OK.
//
bool t5_can_erase_flash()
{
    printf("Erasing the FLASH chips.\r\n");
    if (!t5_boot_erase_command()) {
        printf("Error The ECU's FLASH has not been erased!\r\n");
        return FALSE;
    }
    return TRUE;
}

//
// t5_can_dump_flash
//
// Dumps the FLASH chip BIN file, original.bin to the mbed 'disk'
//
// NOTE the bootloader must be loaded in order to use this function.
//
// Dumping FLASH chips in a T5.5 ECU takes around 35 seconds.
// Dumping T5.2 ECUs should take about half of this time
//
// inputs:    start     start address of the T5.x FLASH

// return:    bool TRUE if all went OK, FALSE if there was an error.
//
bool t5_can_dump_flash(uint32_t start)
{
    printf("Saving the original FLASH BIN file.\r\n");
    FILE *fp = fopen("/local/original.bin", "w");    // Open "original.bin" on the local file system for writing
    if (!fp) {
        perror ("The following error occured");
        return FALSE;
    }
    uint32_t address = start + 5;                  // Mysterious reason for starting at 5 !!!
    char FLASHdata[6];
    printf("  0.00 %% complete.\r");
    while (address < TRIONICLASTADDR) {
        if (!t5_can_read_data(FLASHdata, address)) {
            fclose (fp);
            printf ("Error reading from the CAN bus.\r\n");
            return FALSE;
        }
        address += 6;
        if (fwrite(FLASHdata, 1, 6, fp) != 6) {
            fclose (fp);
            printf ("Error writing to the FLASH BIN file.\r\n");
            return FALSE;
        }
        printf("%6.2f\r", 100*(float)(address-start)/(float)(TRIONICLASTADDR - start) );
    }
    // There are a few more bytes to get because because the bin file is not an exact multiple of 6 bytes!
    // the % (modulo) mathematics function tells us how many bytes there are still to get
    if (!t5_can_read_data(FLASHdata, (TRIONICLASTADDR))) {
        fclose (fp);
        printf ("Error reading from the CAN bus.\r\n");
        return FALSE;
    }
    if (fwrite((FLASHdata + 6 - ((TRIONICLASTADDR - start +1) % 6)), 1, ((TRIONICLASTADDR - start +1) % 6), fp) != ((TRIONICLASTADDR - start +1) % 6)) {
        fclose (fp);
        printf ("Error writing to the FLASH BIN file.\r\n");
        return FALSE;
    }
    printf("100.00 %% complete.\r\n");
    fclose(fp);
    return TRUE;
}


//
// t5_can_send_flash_bin_update
//
// Sends a FLASH update file, modified.bin to the T5 ECU.
// The FLASH update file is stored on the local file system and must be in hex format.
//
// FLASHing a T5.5 ECU takes around 40 seconds. FLASHing T5.2 ECUs should take about half of this time
//
// inputs:    start     start address of the T5.x FLASH

// return:    bool      TRUE if all went OK,
//                      FALSE if the FLASH update failed for some reason.
//
bool t5_can_send_flash_bin_update(uint32_t start)
{
    printf("Programming the FLASH chips.\r\n");
    FILE *fp = fopen("/local/modified.bin", "r");    // Open "modified.bin" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the BIN file MODIFIED.BIN\r\n");
        return FALSE;
    }

    // obtain file size - it should match the size of the FLASH chips:
    fseek (fp , 0 , SEEK_END);
    uint32_t file_size = ftell (fp);
    rewind (fp);

    // read the initial stack pointer value in the BIN file - it should match the value expected for the type of ECU
    uint8_t stack_byte = 0;
    uint32_t stack_long = 0;
    if (!fread(&stack_byte,1,1,fp)) return TERM_ERR;
    stack_long |= (stack_byte << 24);
    if (!fread(&stack_byte,1,1,fp)) return TERM_ERR;
    stack_long |= (stack_byte << 16);
    if (!fread(&stack_byte,1,1,fp)) return TERM_ERR;
    stack_long |= (stack_byte << 8);
    if (!fread(&stack_byte,1,1,fp)) return TERM_ERR;
    stack_long |= stack_byte;
    rewind (fp);

    if (start == T52FLASHSTART && (file_size != T52FLASHSIZE || stack_long != T5POINTER)) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T5.2 ECU :-(\r\n");
        printf("BIN file size: %#10x, FLASH chip size: %#010x, Pointer: %#10x.\r\n", file_size, T52FLASHSIZE, stack_long);
        return TERM_ERR;
    }
    if (start == T55FLASHSTART && (file_size != T55FLASHSIZE || stack_long != T5POINTER)) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T5.5 ECU :-(\r\n");
        printf("BIN file size: %#10x, FLASH chip size: %#010x, Pointer: %#10x.\r\n", file_size, T55FLASHSIZE, stack_long);
        return TERM_ERR;
    }

    char msg[8]; // Construct the bootloader frame for uploading
    uint32_t curr_addr = start; // address to put FLASH data
    uint8_t byte_value = 0;

    printf("  0.00 %% complete.\r");
    while (curr_addr <= TRIONICLASTADDR) {

// send a bootloader address message
        if (!t5_can_send_boot_address(curr_addr, 0x80)) {
            fclose(fp);
            printf("\r\nError sending Block Start message. Address: %08x\r\n",curr_addr);
            return FALSE;
        }
// Construct and send the bootloader frames
// NOTE the last frame sent may have less than 7 real data bytes but 7 bytes are always sent. In this case the unnecessary bytes
// are repeated from the previous frame. This is OK because the T5 ECU knows how many bytes to expect (because the count of bytes
// in the S-Record is sent with the upload address) and ignores any extra bytes in the last frame.
        for (uint8_t i=0; i<0x80; i++) {
            if (!fread(&byte_value,1,1,fp)) {
                fclose(fp);
                printf("\r\nError reading the BIN file MODIFIED.BIN\r\n");
                return FALSE;
            }
            msg[1+(i%7)] = byte_value;
            if (i%7 == 0) msg[0]=i;     // set the index number
            if ((i%7 == 6) || (i == 0x80 - 1))
                if (!t5_can_send_boot_frame(msg)) {
                    fclose(fp);
                    printf("\r\nError sending a Block data message. Address: %08x, Index: %02x\r\n",curr_addr, i);
                    return FALSE;
                }

        }
        curr_addr += 0x80;
        printf("%6.2f\r", 100*(float)(curr_addr - start)/(float)(TRIONICLASTADDR - start) );
    }
    printf("100.00 %% complete.\r\n");
    fclose(fp);
    return TRUE;
}

//
// t5_can_send_flash_s19_update
//
// Sends a FLASH update file, modified.s19 to the T5 ECU.
// The FLASH update file is stored on the local file system and must be in S19 format.
//
// FLASHing a T5.5 ECU takes around 60 seconds. FLASHing T5.2 ECUs should take about half of this time
//
// inputs:    start     start address of the T5.x FLASH

// return:    bool TRUE if all went OK,
//                 FALSE if the FLASH update failed for some reason.
//
bool t5_can_send_flash_s19_update(uint32_t start)
{
    printf("Programming the FLASH chips.\r\n");
    FILE *fp = fopen("/local/modified.s19", "r");    // Open "modified.s19" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the BIN file MODIFIED.S19\r\n");
        return FALSE;
    }
    int c = 0; // for some reason fgetc returns an int instead of a char
    uint8_t count = 0; // count of bytes in the S-record can be 0xFF = 255 in decimal
    uint8_t asize = 0; // 2,3 or 4 bytes in address
    uint32_t address = 0; // address to put S-record
    uint8_t checksum = 0; // checksum check at the end
    char msg[8]; // Construct the bootloader frame for uploading
    bool sent = FALSE;

    while (!sent) {

        do c = fgetc (fp); // get characters until we get an 'S' (throws away \n,\r characters and other junk)
        while (c != 'S' && c != EOF);
//                if (c == EOF) return '\a';
        c = fgetc(fp);        // get the S-record type 1/2/3 5 7/8/9 (Return if EOF reached)
//                if ((c = fgetc(fp)) == EOF) return '\a';        // get the S-record type 1/2/3 5 7/8/9 (Return if EOF reached)
        switch (c) {
            case '0':
                break;                  // Skip over S0 header record

            case '1':
            case '2':
            case '3':
                asize = 1 + c - '0';    // 2, 3 or 4 bytes for address
                address = 0;
// get the number of bytes (in ascii format) in this S-record line
// there must be at least the address and the checksum so return with an error if less than this!
                if ((c = SRecGetByte(fp)) < (asize + 1)) {
                    fclose(fp);
                    printf("Error in S-record address");
                    return FALSE;
                }
                count = c;
                checksum = c;
// get the address
                for (uint8_t i=0; i<asize; i++) {
                    c = SRecGetByte(fp);
                    checksum += c;
                    address <<= 8;
                    address |= c;
                    count--;
                }
// send a bootloader address message - Adding the start address that was supplied (for T5.2/T5.5)
                if (!t5_can_send_boot_address((address + start), (count - 1))) {
                    fclose(fp);
                    printf("Error sending CAN message");
                    return FALSE;
                }
// get and send the bootloader frames for this S-record
// NOTE the last frame sent may have less than 7 real data bytes but 7 bytes are always sent. In this case the unnecessary bytes
// are repeated from the previous frame. This is OK because the T5 ECU knows how many bytes to expect (because the count of bytes
// in the S-Record is sent with the upload address) and ignores any extra bytes in the last frame.
                for (uint8_t i=0; i<count-1; i++) {
                    c = SRecGetByte(fp);
                    checksum += c;
                    msg[1+(i%7)] = c;
                    if (i%7 == 0) msg[0]=i;     // set the index number
                    if ((i%7 == 6) || (i == count - 2))
                        if (!t5_can_send_boot_frame(msg)) {
                            fclose(fp);
                            printf("Error sending CAN message");
                            return FALSE;
                        }
                }
// get the checksum
                if ((checksum += SRecGetByte(fp)) != 0xFF) {
                    fclose(fp);
                    printf("Error in S-record, checksum = %2x\r\n", checksum);
                    return FALSE;
                }
                break;

            case '5':
                break;                  // Skip over S5 record types

            case '7':
            case '8':
            case '9':
                sent = TRUE;
                break;

// Some kind of invalid S-record type so break
            default:
                printf("oops - didn't recognise that S-Record\r\n");
                fclose(fp);
                return FALSE;
        }
    }
    fclose(fp);
    return TRUE;
}

//
// t5_can_get_last_address
//
// Sends a C3 Message to the T5 ECU. The reply should contain the last used FLASH address
// which is always 0x0007FFFF
//
// The last 2 bytes of the message might be useful to work out whether or not the bootloader
// has been loaded. The 'mode' value will be 0x0808 when the bootloader is running or 0x89b8
// when it isn't.
//
// inputs:    none
// return:    bool TRUE if all went OK,
//
bool t5_can_get_last_address()
{
    uint32_t last_address = 0;
    uint16_t mode = 0;
    if (!t5_boot_c3_command(&last_address, &mode)) {
        printf("Error trying to find out the ECU's last address and mode!\r\n");
        return FALSE;
    }
    printf("The Last used FLASH address is: %#010x, mode %#06x\r\n", last_address, mode);
    return TRUE;
}

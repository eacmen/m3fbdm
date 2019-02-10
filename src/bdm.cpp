/*******************************************************************************

bdm.cpp
(c) 2010 by Sophie Dexter

BDM functions for Just4Trionic by Just4pLeisure

A derivative work based on:
//-----------------------------------------------------------------------------
//    Firmware for USB BDM v2.0
//    (C) johnc, 2009
//    $id$
//-----------------------------------------------------------------------------

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "bdm.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// LED constants and macros
#define LED_ONTIME          5               ///< LED 'on' time, ms    
#define LED_TCNT            (255 - (unsigned char)((unsigned long)(LED_ONTIME * 1000L)  \
                            / (1000000L / (float)((unsigned long)XTAL_CPU / 1024L))))

// command characters
#define CMDGROUP_ADAPTER    'a'             ///< adapter commands 
#define CMD_VERSION         'v'             ///< firmware version
#define CMD_PINSTATUS       's'             ///< momentary status of BDM pins
#define CMD_BKPTLOW         '1'             ///< pull BKPT low
#define CMD_BKPTHIGH        '2'             ///< pull BKPT high
#define CMD_RESETLOW        '3'             ///< pull RESET low
#define CMD_RESETHIGH       '4'             ///< pull RESET high
#define CMD_BERR_LOW        '5'             ///< pull BERR low
#define CMD_BERR_HIGH       '6'             ///< pull BERR high
#define CMD_BERR_INPUT      '7'             ///< make BERR an input


#define CMDGROUP_MCU        'c'             ///< target MCU management commands
#define CMD_STOPCHIP        'S'             ///< stop
#define CMD_RESETCHIP       'R'             ///< reset
#define CMD_RUNCHIP         'r'             ///< run from given address
#define CMD_RESTART         's'             ///< restart
#define CMD_STEP            'b'             ///< step

#define CMDGROUP_FLASH      'f'
#define CMD_GETVERIFY       'v'             ///< gets verification status
#define CMD_SETVERIFY       'V'             ///< sets verification on/off
#define CMD_DUMP            'd'             ///< dumps memory contents
#define CMD_ERASE           'E'             ///< erase entire flash memory            
#define CMD_WRITE           'w'             ///< writes to flash memory

#define CMDGROUP_MEMORY     'm'             ///< target MCU memory commands
#define CMD_READBYTE        'b'             ///< read byte from memory
#define CMD_READWORD        'w'             ///< read word (2 bytes) from memory
#define CMD_READLONG        'l'             ///< read long word (4 bytes) from memory
#define CMD_DUMPBYTE        'z'             ///< dump byte from memory
#define CMD_DUMPWORD        'x'             ///< dump word from memory
#define CMD_DUMPLONG        'c'             ///< dump long word from memory
#define CMD_WRITEBYTE       'B'             ///< write byte to memory
#define CMD_WRITEWORD       'W'             ///< write word to memory
#define CMD_WRITELONG       'L'             ///< write long word to memory
#define CMD_FILLBYTE        'f'             ///< fill byte in memory
#define CMD_FILLWORD        'F'             ///< fill word in memory
#define CMD_FILLLONG        'M'             ///< fill long word in memory 

#define CMDGROUP_REGISTER   'r'             ///< register commands
#define CMD_READSYSREG      'r'             ///< read system register
#define CMD_WRITESYSREG     'W'             ///< write system register
#define CMD_READADREG       'a'             ///< read A/D register
#define CMD_WRITEADREG      'A'             ///< write A/D register
#define CMD_DISPLAYREGS     'd'             ///< BD32 like display all registers

#define CMDGROUP_TRIONIC    'T'
#define CMD_TRIONICDUMP     'D'             ///< dumps memory contents
#define CMD_TRIONICWRITE    'F'             ///< writes to flash memory

// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer
static uint32_t cmd_addr;                   ///< address (optional)
static uint32_t cmd_value;                  ///< value    (optional)
static uint32_t cmd_result;                 ///< result
static bool verify_flash = 1;

// private functions
uint8_t execute_bdm_cmd();

void bdm_show_help();
void bdm_show_full_help();

// command argument macros
#define CHECK_ARGLENGTH(len) \
    if (cmd_length != len + 2) \
        return TERM_ERR

#define GET_NUMBER(target, offset, len)    \
    if (!ascii2int(target, cmd_buffer + 2 + offset, len)) \
        return TERM_ERR


void bdm()
{

    bdm_show_help();
    // set up LED pins
//    SETBIT(LED_DIR, _BV(LED_ERR) | _BV(LED_ACT));
    // set up USB_RD and USB_WR pins
//    SETBIT(USB_RXTX_DIR, _BV(USB_RD) | _BV(USB_WR));

    // enable interrupts
//    sei();

    // load configuration from EEPROM
//    verify_flash = eeprom_read_byte(&ee_verify);

// Set some initial values to help with checking if the BDM connector is plugged in
    //PIN_PWR.mode(PullDown);
//    PIN_NC.mode(PullUp);
//    PIN_DS.mode(PullUp);
    PIN_FREEZE.mode(PullUp);
    PIN_DSO.mode(PullUp);

    verify_flash = true;

    // main loop
    *cmd_buffer = '\0';
    char ret;
    char rx_char;
    while (true) {
        // read chars from USB
        if (pc.readable()) {
            // turn Error LED off for next command
            led2 = 0;
            rx_char = pc.getc();
            switch (rx_char) {
                    // 'ESC' key to go back to mbed Just4Trionic 'home' menu
                case '\e':
                    reset_chip();
                    return;
                    // end-of-command reached
                case TERM_OK :
                    // execute command and return flag via USB
                    ret = execute_bdm_cmd();
                    pc.putc(ret);
                    // reset command buffer
                    *cmd_buffer = '\0';
                    // light up LED
//                    ret == TERM_OK ? led_on(LED_ACT) : led_on(LED_ERR);
                    ret == TERM_OK ? led1 = 1 : led2 = 1;
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
uint8_t execute_bdm_cmd()
{
    uint8_t cmd_length = strlen(cmd_buffer);
    char cmd = *(cmd_buffer + 1);

    // command groups
    switch (*cmd_buffer) {
            // adapter commands
        case CMDGROUP_ADAPTER:
            CHECK_ARGLENGTH(0);
            switch (cmd) {
                    // get firmware version
                case CMD_VERSION:
                    printf("%02x", FW_VERSION_MAJOR);
                    printf("%02x", FW_VERSION_MINOR);
                    printf("\r\n");
                    return TERM_OK;

                    // get momentary status of BDM pins 0...5 (for debugging)
                case CMD_PINSTATUS:
//                    printf("%02x", (BDM_PIN & 0x3f));
                    printf("PWR %d, ", PIN_PWR.read());
//                    printf("NC %d, ", PIN_NC.read());
//                    printf("DS %d, ", PIN_DS.read());
                    printf("FREEZE %d, ", PIN_FREEZE.read());
                    printf("DSO %d, ", PIN_DSO.read());
                    printf("\r\n");
                    return TERM_OK;

                    // pull BKPT low
                case CMD_BKPTLOW:
                    return bkpt_low();

                    // pull BKPT high
                case CMD_BKPTHIGH:
                    return bkpt_high();

                    // pull RESET low
                case CMD_RESETLOW:
                    return reset_low();

                    // pull RESET high
                case CMD_RESETHIGH:
                    return reset_high();

                    // pull BERR low
                case CMD_BERR_LOW:
                    return berr_low();

                    // pull BERR high
                case CMD_BERR_HIGH:
                    return berr_high();

                    // make BERR an input
                case CMD_BERR_INPUT:
                    return berr_input();
            }
            break;

            // MCU management
        case CMDGROUP_MCU:
            switch (cmd) {
                    // stop target MCU
                case CMD_STOPCHIP:
                    return stop_chip();

                    // reset target MCU
                case CMD_RESETCHIP:
                    return reset_chip();

                    // run target MCU from given address
                case CMD_RUNCHIP:
                    CHECK_ARGLENGTH(8);
                    GET_NUMBER(&cmd_addr, 0, 8);
                    return run_chip(&cmd_addr);

                    // restart
                case CMD_RESTART:
                    return restart_chip();

                    // step
                case CMD_STEP:
                    return step_chip();
            }
            break;

            // Trionic dumping and flashing
        case CMDGROUP_FLASH:
            switch (cmd) {
                    // get verification flag
                case CMD_GETVERIFY:
                    CHECK_ARGLENGTH(0);
                    printf("%02x", (uint8_t)verify_flash);
                    printf("\r\n");
                    return TERM_OK;

                    // set verification flag
                case CMD_SETVERIFY:
                    CHECK_ARGLENGTH(2);
                    GET_NUMBER(&cmd_addr, 0, 2);
                    verify_flash = (bool)cmd_addr;
//                    eeprom_write_byte(&ee_verify, verify_flash);
                    return TERM_OK;

                    // dump flash contents as block
                case CMD_DUMP:
                    CHECK_ARGLENGTH(16);
                    GET_NUMBER(&cmd_addr, 0, 8);
                    GET_NUMBER(&cmd_value, 8, 8);
                    return dump_flash(&cmd_addr, &cmd_value);

                    // erase entire flash memory
                case CMD_ERASE:
                    CHECK_ARGLENGTH(22);
                    GET_NUMBER(&cmd_addr, 6, 8);
                    GET_NUMBER(&cmd_value, 14, 8);
                    return erase_flash(cmd_buffer + 2, &cmd_addr, &cmd_value);

                    // write data block to flash memory
                case CMD_WRITE:
                    CHECK_ARGLENGTH(14);
                    GET_NUMBER(&cmd_addr, 6, 8);
                    return write_flash(cmd_buffer + 2, &cmd_addr);
            }
            break;

            // memory
        case CMDGROUP_MEMORY:
            if (cmd != CMD_FILLBYTE && cmd != CMD_FILLWORD &&
                    cmd != CMD_FILLLONG && cmd != CMD_DUMPBYTE && cmd != CMD_DUMPWORD &&
                    cmd != CMD_DUMPLONG) {
                // get memory address
                if (cmd_length < 10 || !ascii2int(&cmd_addr, cmd_buffer + 2, 8)) {
                    // broken parametre
                    return TERM_ERR;
                }

                // get optional value
                if (cmd_length > 10 &&
                        !ascii2int(&cmd_value, cmd_buffer + 10, cmd_length - 10)) {
                    // broken parametre
                    return TERM_ERR;
                }
            }

            switch (cmd) {
                    // read byte
                case CMD_READBYTE:
                    if (cmd_length != 10 ||
                            memread_byte((uint8_t*)(&cmd_result), &cmd_addr) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%02x", (uint8_t)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // read word
                case CMD_READWORD:
                    if (cmd_length != 10 ||
                            memread_word((uint16_t*)(&cmd_result), &cmd_addr) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%04X", (uint16_t)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // read long word
                case CMD_READLONG:
                    if (cmd_length != 10 ||
                            memread_long(&cmd_result, &cmd_addr) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%08X", (unsigned int)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // dump byte
                case CMD_DUMPBYTE:
                    if (cmd_length != 2 ||
                            memdump_byte((uint8_t*)(&cmd_result)) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%02x", (uint8_t)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // dump word
                case CMD_DUMPWORD:
                    if (cmd_length != 2 ||
                            memdump_word((uint16_t*)(&cmd_result)) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%04X", (uint16_t)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // dump long word
                case CMD_DUMPLONG:
                    if (cmd_length != 2 ||
                            memdump_long(&cmd_result) != TERM_OK) {
                        return TERM_ERR;
                    }
                    printf("%08X", (unsigned int)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // write byte
                case CMD_WRITEBYTE:
                    return (cmd_length == 12 &&
                            memwrite_byte(&cmd_addr, cmd_value) == TERM_OK) ?
                           TERM_OK : TERM_ERR;

                    // write word
                case CMD_WRITEWORD:
                    return (cmd_length == 14 &&
                            memwrite_word(&cmd_addr, cmd_value) == TERM_OK) ?
                           TERM_OK : TERM_ERR;

                    // write long word
                case CMD_WRITELONG:
                    return (cmd_length == 18 &&
                            memwrite_long(&cmd_addr, &cmd_value) == TERM_OK) ?
                           TERM_OK : TERM_ERR;

                    // fill byte
                case CMD_FILLBYTE:
                    if (cmd_length != 4 || !ascii2int(&cmd_value, cmd_buffer + 2, 2)) {
                        return TERM_ERR;
                    }
                    return (memfill_byte(cmd_value));

                    // fill word
                case CMD_FILLWORD:
                    if (cmd_length != 6 || !ascii2int(&cmd_value, cmd_buffer + 2, 4)) {
                        return TERM_ERR;
                    }
                    return (memfill_word(cmd_value));

                    // fill long word
                case CMD_FILLLONG:
                    if (cmd_length != 10 || !ascii2int(&cmd_value, cmd_buffer + 2, 8)) {
                        return TERM_ERR;
                    }
                    return (memfill_long(&cmd_value));
            }
            break;

            // registers
        case CMDGROUP_REGISTER:
            // get register code
            if (cmd_length == 2) {
                if (cmd != CMD_DISPLAYREGS) {
                    // broken parametre
                    printf("err %s line: %d, cmd_length %d\r\n", __FILE__, __LINE__, cmd_length);
                    return TERM_ERR;
                }
            } else if (cmd_length < 4 || !ascii2int(&cmd_addr, cmd_buffer + 3, 1)) {
                // broken parametre
                printf("err %s line: %d, cmd_length %d\r\n", __FILE__, __LINE__, cmd_length);
                return TERM_ERR;
            }
            // get optional value
            if (cmd_length > 4 && !ascii2int(&cmd_value, cmd_buffer + 4, 8)) {
                // broken parametre
                printf("err %s line: %d, cmd_length %d\r\n", __FILE__, __LINE__, cmd_length);
                return TERM_ERR;
            }

            switch (cmd) {
                    // read system register
                case CMD_READSYSREG:
                    if (cmd_length != 4 ||
                            sysreg_read(&cmd_result, (uint8_t)cmd_addr) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("%08X", (unsigned int)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // write system register
                case CMD_WRITESYSREG:
                    return (cmd_length == 12 &&
                            sysreg_write((uint8_t)cmd_addr, &cmd_value) == TERM_OK) ?
                           TERM_OK : TERM_ERR;

                    // read A/D register
                case CMD_READADREG:
                    if (cmd_length != 4 ||
                            adreg_read(&cmd_result, (uint8_t)cmd_addr) != TERM_OK) {
                        printf("err %s line: %d, cmd_length %d\r\n", __FILE__, __LINE__, cmd_length);
                        return TERM_ERR;
                    }
                    printf("%08X", (unsigned int)cmd_result);
                    printf("\r\n");
                    return TERM_OK;

                    // write A/D register
                case CMD_WRITEADREG:
                    return (cmd_length == 12 &&
                            adreg_write((uint8_t)cmd_addr, &cmd_value) == TERM_OK) ?
                           TERM_OK : TERM_ERR;

                    // Display all registers
                case CMD_DISPLAYREGS:
                    printf(" Register Display\r\n");
                    printf(" D0-7");
                    for (uint8_t i = 0; i < 8; i++) {
                        if (adreg_read(&cmd_result, i) != TERM_OK) {
                            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                            return TERM_ERR;
                        }
                        printf(" %08X", (unsigned int)cmd_result);
                    }
//
                    printf("\r\n");
                    printf(" A0-7");
                    for (uint8_t i = 8; i < 16; i++) {
                        if (adreg_read(&cmd_result, i) != TERM_OK) {
                            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                            return TERM_ERR;
                        }
                        printf(" %08X", (unsigned int)cmd_result);
                    }
                    printf("\r\n");
//
                    if (sysreg_read(&cmd_result, 0x0) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("  RPC %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0xC) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("     USP %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0xE) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("     SFC %08X      SR 10S--210---XNZVC\r\n", (unsigned int)cmd_result);
//
                    if (sysreg_read(&cmd_result, 0x1) != TERM_OK) {

                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("  CPC %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0xD) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("     SSP %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0xF) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("     DFC %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0xB) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("    %04X ", (uint16_t)cmd_result);
                    for (uint32_t i = 0; i < 16; i++) {
                        cmd_result & (1 << (15-i)) ? printf("1") : printf("0");
                    }
                    printf("\r\n");
//
                    if (sysreg_read(&cmd_result, 0xA)!= TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("  VBR %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0x8)!= TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("   ATEMP %08X", (unsigned int)cmd_result);
                    if (sysreg_read(&cmd_result, 0x9) != TERM_OK) {
                        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
                        return TERM_ERR;
                    }
                    printf("     FAR %08X\r\n", (unsigned int)cmd_result);
//
//                    printf(" !CONNNECTED:%d", PIN_NC.read());
                    printf(" POWER:%d", PIN_PWR.read());
                    printf(" !RESET:%d", PIN_RESET.read());
                    printf(" !BUSERR:%d", PIN_BERR.read());
                    printf(" BKPT/DSCLK:%d", PIN_BKPT.read());
                    printf(" FREEZE:%d", PIN_FREEZE.read());
                    printf(" DSO:%d", PIN_DSO.read());
                    printf(" DSI:%d", PIN_DSI.read());
                    printf("\r\n");
//
                    return TERM_OK;
            }
            break;

            // Trionic dumping and flashing
        case CMDGROUP_TRIONIC:

            switch (cmd) {

                    // dump flash contents to a bin file
                case CMD_TRIONICDUMP:
                    CHECK_ARGLENGTH(0);
                    return dump_trionic();

                    // write data block to flash memory
                case CMD_TRIONICWRITE:
                    CHECK_ARGLENGTH(0);
                    return flash_trionic();
            }

            // show help for BDM commands
        case 'H':
            bdm_show_full_help();
            return TERM_OK;
        case 'h':
            bdm_show_help();
            return TERM_OK;
        default:
            bdm_show_help();
    }

// unknown command
    return TERM_ERR;
}

void bdm_show_help()
{
    printf("Just4Trionic BDM Command Menu\r\n");
    printf("=============================\r\n");
    printf("TD - and DUMP T5 FLASH BIN file\r\n");
    printf("TF - FLASH the update file to the T5\r\n");
//    printf("TF - FLASH the update file to the T5 (and write SRAM)\r\n");
//    printf("Tr - Read SRAM adaption (not done).\r\n");
//    printf("Tw - Write SRAM adaptation (not done).\r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("h  - Show this help menu\r\n");
    printf("\r\n");
    return;
}
void bdm_show_full_help()
{
    printf("Just4Trionic BDM Command Menu\r\n");
    printf("=============================\r\n");
    printf("TD - and DUMP T5 FLASH BIN file\r\n");
    printf("TF - FLASH the update file to the T5\r\n");
//    printf("TF - FLASH the update file to the T5 (and write SRAM)\r\n");
//    printf("Tr - Read SRAM adaption (not done).\r\n");
//    printf("Tw - Write SRAM adaptation (not done).\r\n");
    printf("\r\n");
    printf("Adapter Commands - a\r\n");
    printf("====================\r\n");
    printf("av - display firmware version\r\n");
    printf("as - display status of BDM pins\r\n");
    printf("a1 - pull BKPT low\r\n");
    printf("a2 - pull BKPT high\r\n");
    printf("a3 - pull RESET low\r\n");
    printf("a4 - pull RESET high\r\n");
    printf("a5 - pull BERR low\r\n");
    printf("a6 - pull BERR high\r\n");
    printf("a7 - make BERR an input\r\n");
    printf("\r\n");
    printf("MCU Management Commands - c\r\n");
    printf("===========================\r\n");
    printf("cS - stop MCU\r\n");
    printf("cR - reset MCU\r\n");
    printf("cr - run from given address\r\n");
    printf("     e.g. crCAFEBABE run from address 0xcafebabe\r\n");
    printf("cs - restart MCU\r\n");
    printf("cb - step MCU\r\n");
    printf("\r\n");
    printf("MCU FLASH Commands - f\r\n");
    printf("======================\r\n");
    printf("fv - gets verification status (always verify)\r\n");
    printf("fV - sets verification on/off (always on)\r\n");
    printf("fd - DUMPs memory contents\r\n");
    printf("     e.g. fdSSSSSSSSEEEEEEEE S...E... start and end addresses\r\n");
    printf("     e.g. fd0000000000020000 to DUMP T5.2\r\n");
    printf("     e.g. fd0000000000040000 to DUMP T5.5\r\n");
    printf("     e.g. fd0000000000080000 to DUMP T7\r\n");
    printf("fE - Erases entire FLASH memory\r\n");
    printf("     e.g. fETTTTTTSSSSSSSSEEEEEEEE T...S...E type, start and end addresses\r\n");
    printf("     e.g. fE28f0100000000000020000 erase 28F512 in T5.2\r\n");
    printf("     e.g. fE28f0100000000000040000 erase 28F010 in T5.5\r\n");
    printf("     e.g. fE29f0100000000000040000 erase 29F010 in T5.5 (addresses not used)\r\n");
    printf("     e.g. fE29f4000000000000080000 erase 29F400 in T7 (addresses not used)\r\n");
    printf("fw - writes to FLASH memory\r\n");
    printf("     Write a batch of long words to flash from a start address\r\n");
    printf("     followed by longwords LLLLLLLL, LLLLLLLL etc\r\n");
    printf("     Send a break character to stop when doneg");
    printf("     e.g. fwTTTTTTSSSSSSSS T...S... type and start address\r\n");
    printf("     e.g. fw28f01000004000 start at address 0x004000 T5.2/5.5\r\n");
    printf("     e.g. fw29f01000004000 start at address 0x004000 T5.5 with 29F010\r\n");
    printf("     e.g. fw29f40000004000 start at address 0x004000 T7\r\n");
    printf("\r\n");
    printf("MCU Memory Commands - m\r\n");
    printf("=======================\r\n");
    printf("mbAAAAAAAA - read byte from memory at 0xaaaaaaaa\r\n");
    printf("mwAAAAAAAA - read word (2 bytes) from memory\r\n");
    printf("mlAAAAAAAA - read long word (4 bytes) from memory\r\n");
    printf("mz - dump byte from the next memory address\r\n");
    printf("mx - dump word from the next memory address\r\n");
    printf("mc - dump long word from the next memory address\r\n");
    printf("mBAAAAAAAADD - write byte 0xdd to memory 0xaaaaaaaa\r\n");
    printf("mWAAAAAAAADDDD - write word 0xdddd to memory 0xaaaaaaaa\r\n");
    printf("mLAAAAAAAADDDDDDD - write long word 0xdddddddd to memory 0xaaaaaaaa\r\n");
    printf("mfDD - fill byte 0xdd to next memory address\r\n");
    printf("mFDDDD - fill word 0xdddd to next memory address\r\n");
    printf("mMDDDDDDDD - fill long word 0xdddddddd to next memory address\r\n");
    printf("\r\n");
    printf("MCU Register Commands - r\r\n");
    printf("=========================\r\n");
    printf("rd     Display all registers (like BD32)\r\n");
    printf("rrRR - read system register RR\r\n");
    printf("       00 - RPC, 01 - PCC, 0B - SR, 0C - USP, 0D - SSP\r\n");
    printf("       0e - SFC, 0f - DFC, 08 - ATEMP, 09 - FAR, 0a - VBR\r\n");
    printf("rWRRCAFEBABE - write 0xcafebabe system register RR\r\n");
    printf("raAD - read A/D register 0x00-07 D0-D7, 0x08-15 A0-A7\r\n");
    printf("rAADCAFEBABE - write 0xcafebabe to A/D register \r\n");
    printf("\r\n");
    printf("'ESC' - Return to Just4Trionic Main Menu\r\n");
    printf("\r\n");
    printf("H  - Show this help menu\r\n");
    printf("\r\n");
    return;
}

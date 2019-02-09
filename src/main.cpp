/*******************************************************************************

Just4Trionic by Just4pLeisure
*****************************
(c) 2010 by Sophie Dexter

Whilst I have written this program myself I could not have done it without
a lot of help and the original ideas and programs written by:
Dilemma - Author of the Trionic Suite software programs and general Guru
http://trionic.mobixs.eu/ and http://www.ecuproject.com.
General Failure - Author of the T5CANlib software and regular contributor at
http://www.ecuproject.com.
Tomi Liljemark - Lots of information and programs about the Saab CAN bus
http://pikkupossu.1g.fi/tomi/projects/projects.html.
Scott Howard - Author of the BDM software.
Janis Silins - Valued contributor at http://www.ecuproject.com and creator of
USBBDM. For sharing his BDM software (which also had a very useful method of
entering commands)
Plus inspiration and ideas from many others...

Sophie x

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

********************************************************************************

Version 1.6 (04/2016)

Fixed since Version 1.5:
    BDM
        Improvements to CPU register content display
        T8 erase timeout period to 200 seconds (was incorrectly only 20 seconds)

Added since Version 1.5:
    T7 CAN functions:
        Separate FLASHing algorithms for I and P-BUS connections
        Completely safe, but slow, I-BUS algorithm
        Pseudo 'recovery' FLASHing function can be used to retry a failed
        FLASH with safe, slow algorithm
        
Changes since Version 1.5:
    Additional debug messages
    T7 CAN functions:
        Code refactoring to make file handling simpler

Version 1.5 (04/2015)

Added since Version 1.4
    T8 CAN functions: DUMP, FLASH and RECOVERY
        Using modified T8Bootloaders that are slightly faster :-)
    Algorithms for different types of 5 volt replacement FLASH Chips in T5 ECUs
        AMD, ST and AMIC 29F010 types
        SST 39F010
        ATMEL 29C512 and 29C010
        
Changes since Version 1.4
    BDM FLASHing uses BD32 like Target Resident Drivers.
        Based on my 'universal scripts'
        BDM FLASHing much faster now
    Always use slow BDM clock function (bdm_clk_slow) for reliability
        Even when using the slow method it only takes 8 seconds
        longer to FLASH a T8 ECU.
        mbed library and compiler changes make it difficult to
        maintain the faster bdm functions that rely on software
        delay loops for timing.
    The file extension of the file to be FLASHed is now the more usual 'BIN'.
        i.e. 'modified.bin' instead of 'modified.hex'

********************************************************************************

Version 1.4 (07/2013)

Added since Verion 1.3
    Progress indication shown as a percentage, slightly slower but more informative
    T8 BDM Functionality

Changes since Verion 1.3
    T7 CAN FLASH algorithm changed to only send 4 Bytes at a time
        Slightly slower but more reliable especially with I-Bus connection

********************************************************************************

Version 1.3 (06/2011) - Basic T7 CAN support and faster T5 CAN bootloader

Changes since Verion 1.2
    New T7 CAN menu provides some very basic T7 CAN DUMP and FLASH functions
    T5 CAN uses Mybooty version 2 for 1 Mbps CAN speed
        Get MyBooty version 2.x from forum.ecuproject.com
        Does not work with version MyBooty 1.x

********************************************************************************

Version 1.2 (12/2010) - Only a very small update

Fixed since Version 1.1:
    My method of detecting the FLASH type didn't work for T7 ECUs
        Now that that I have corrected this bug it is possible to
        FLASH Trionic 7 ECUs using the BDM connection
        See the 'get_flash_id' function in bdmtrionic.cpp

Changes since Verion 1.1
    I have removed everything to do with the BDM DS connection
    I have changed the mbed pin number for the BDM DSO connection
        Now all BDM connections are part of the same MBED 'port'
            Being part of one port makes it possible to change all
            connections simulataneously so speeding up BDM
        See 'interfaces.cpp' for details of how the mbed pins connect.
            Uglybug's wiring circuit has these connections (thanks Uglybug)

********************************************************************************

Version 1.1 (09/2010) - Still very crude way of doing things

Additions since Version 1:
    The BDM interface is now working
        Based on Janis Silin's BDM software with modifications and additions
            Detect which type of FLASH chip is fitted to work out type of ECU
            Modifications to FLASH algorithms - I think my algorithms are
            closer to the datasheet methods. Still to do:
                Separate pulse counters for 28Fxxx erase
                DQ7 and DQ5 checking method for 29Fxxx FLASH
        Works for T5.5 ECUs with 28F010 and 29F010 chips
        Probably works with T5.2 ECUs (chip detection method)
        MAY work with T7 ECUs ('prep' method may need changes - I can't test T7)
        NOTE: Some of Janis' original BDM commands may not work, or at least
        not as originally intended
    Lawicell CAN232 interface partially working
        Only a few Lawicell message types to open/close, set speed and write
    Trionic5 CAN functions
        All-in-one 'D' and 'F' commands to DUMP and FLASH BIN files
            Lots of checking for errors, either works or says it failed
            No need to interpret the cryptic CAN messages anymore
        Should now work for T5.2 and T5.5 ECUs
        Detects FLASH chip type and works out which ECU is connected T5.2/5.5

********************************************************************************

Version 1 (04/2010)- The basic CAN functions are working

I have decided to 'release' this software somewhat prematurely because the FLASH
chips in my spare ECU have 'died' and I don't know when I will be able to do
carry on improving and 'polishing' it. This way others will be able to use and
enhance it without having to wait for me.

For now, only option '5' Trionic ECU CAN interface is working. BDM and Lawicell
CAN232 functions are dummies. The intention is to build a complete suite of CAN
software for Trionic5, 7 and 8 ECU types as well as adding a BDM interface to
make an 'all-in-one' USB programming tool.

To make this you will need an mbed system and the CAN circuit from this page:
http://mbed.org/projects/cookbook/wiki/CanBusExample1

Some ideas for the truly creative and adventurous of you is to make a gizmo that
doesn't even need to be connected to a laptop or PC to use, maybe even a self-
contained vesion of Dilemma's CarPC using ideas from this pages:

http://mbed.org/projects/cookbook/wiki/PS2Keyboard
http://mbed.org/projects/cookbook/wiki/PS2Mouse
http://mbed.org/projects/cookbook/wiki/MobileLCD
http://mbed.org/projects/cookbook/wiki/SDCard

*******************************************************************************/

#include "mbed.h"
//
#include "common.h"
#include "bdm.h"
#include "can232.h"
#include "t5can.h"
#include "t7can.h"
#include "t8can.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer

// private functions
uint8_t execute_just4trionic_cmd();
void show_just4trionic_help();

int main()
{
    // fast serial speed
    //pc.baud(921600);
    pc.baud(115200);

    // the address of the function to be attached (leds_off) and the interval (0.1 seconds)
    // This 'ticker' turns off the activity LEDs so that they don't stay on if something has gone wrong
    ticker.attach(&leds_off, 0.1);

    // clear incoming buffer
    // sometimes TeraTerm gets 'confused'. johnc does this in his code
    // hopefully this will fix the problem
    // unfortunately it doesn't, but it seems like a good idea
    char rx_char;
    while (pc.readable())
        rx_char = pc.getc();

    show_just4trionic_help();

    // main loop
    *cmd_buffer = '\0';
    char ret;
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
                    // end-of-command reached
                case TERM_OK :
                    // execute command and return flag via USB
                    timer.reset();
                    timer.start();
                    ret = execute_just4trionic_cmd();
                    show_just4trionic_help();
                    pc.putc(ret);
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
uint8_t execute_just4trionic_cmd()
{


//    uint8_t cmd_length = strlen(cmd_buffer);
    // command groups
    switch (*cmd_buffer) {
//            CHECK_ARGLENGTH(0);
        case 'b':
        case 'B':
            bdm();
            return TERM_OK;
        case 'o':
        case 'O':
            can232();
            return TERM_OK;
        case '5':
            t5_can();
            return TERM_OK;
        case '7':
            t7_can();
            return TERM_OK;
        case '8':
            t8_can();
            return TERM_OK;
        case 'h':
        case 'H':
            return TERM_OK;
        default:
            break;
    }

// unknown command
    return TERM_ERR;
}

void show_just4trionic_help()
{
#ifdef DEBUG
    printf("*************************\r\n");
    printf("** D E B U G B U I L D **\r\n");
    printf("*************************\r\n");
#endif
    printf("=========================\r\n");
    printf("Just4Trionic Release %d.%d\r\n", FW_VERSION_MAJOR, FW_VERSION_MINOR);
    printf("=========================\r\n");
    printf("Modes Menu\r\n");
    printf("=========================\r\n");
    printf("b/B - Enter BDM mode\r\n");
    printf("o/O - Enter Lawicel CAN mode\r\n");
    printf("5   - Enter Trionic5 CAN mode\r\n");
    printf("7   - Enter Trionic7 CAN mode\r\n");
    printf("8   - Enter Trionic8 CAN mode\r\n");
    printf("\r\n");
    printf("h/H - show this help menu\r\n");
    printf("\r\n");
    return;
}
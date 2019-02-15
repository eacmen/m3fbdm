/*
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

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.
*/

#include "mbed.h"
//
#include "common.h"
#include "bdm.h"

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
        if (pc.readable()) {
            // turn Error LED off for next command
            led2 = 0;
            rx_char = pc.getc();
            switch (rx_char) {
                    // end-of-command reached
                case CR :
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
                    ret == TERM_OK ? led1 = 1 : led2 = 1;
                    break;
                    // another command char
                case LF:
                    // nop
                    break ;
                default:
                    // store in buffer if space permits
                    if (StrLen(cmd_buffer) < CMD_BUF_LENGTH - 1) {
                        pc.putc(rx_char);
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
    printf("Running command: %s\n", cmd_buffer) ;

//    uint8_t cmd_length = strlen(cmd_buffer);
    // command groups
    switch (*cmd_buffer) {
//            CHECK_ARGLENGTH(0);
        case 'b':
        case 'B':
            bdm( );
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
    printf("m3fbdm Release %d.%d\r\n", FW_VERSION_MAJOR, FW_VERSION_MINOR);
    printf("=========================\r\n");
    printf("Modes Menu\r\n");
    printf("=========================\r\n");
    printf("b/B - Enter BDM mode\r\n");
    printf("\r\n");
    printf("h/H - show this help menu\r\n");
    printf("\r\n");
    return;
}
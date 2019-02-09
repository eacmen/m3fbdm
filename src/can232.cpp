/*******************************************************************************

can232.cpp
(c) 2010 by Sophie Dexter
portions (c) 2009, 2010 by Janis Silins (johnc)

Lawicel CAN232 type functions for Just4Trionic by Just4pLeisure

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "can232.h"

// constants
#define CMD_BUF_LENGTH      32              ///< command buffer size

// command characters

#define CMD_CLOSE           'C'             ///< Close the CAN device
#define CMD_OPEN            'O'             ///< Open the CAN device (do this before an S/s command)

#define CMD_PRESET_SPEED    'S'             ///< Sn: set preconfigured speeds 
#define CMD_SPEED_0         '0'             ///< 10 kbits (or 47,619 bits with 's')
#define CMD_SPEED_1         '1'             ///< 20 kbits (or 500 kbits with 's')
#define CMD_SPEED_2         '2'             ///< 50 kbits (or 615 kbits with 's')
#define CMD_SPEED_3         '3'             ///< 100 kbits
#define CMD_SPEED_4         '4'             ///< 125 kbits
#define CMD_SPEED_5         '5'             ///< 250 kbits
#define CMD_SPEED_6         '6'             ///< 500 kbits
#define CMD_SPEED_7         '7'             ///< 800 kbits
#define CMD_SPEED_8         '8'             ///< 1 mbits
#define CMD_DIRECT_SPEED    's'             ///< sxxyy: set the CAN bus speed registers directly
///< xx: BTR0 register setting
///< yy: BTR1 register setting

#define CMD_SEND_11BIT      't'             ///< tiiildd..: send 11 bit id CAN frame
///< iii: identfier 0x0..0x7ff
///< l: Number of data bytes in CAN frame 0..8
///< dd..: data byte values 0x0..0xff (l pairs)
#define CMD_SEND_29BIT      'T'             ///< Tiiiiiiiildd..: send 29 bit id CAN frame
///< iiiiiiii: identifier 0x0..0x1fffffff
///< l: Number of data bytes in CAN frame 0..8
///< dd..: data byte values 0x0..0xff (l pairs)


#define CMD_READ_FLAGS      'F'             ///< Read flags !?!

#define CMD_FILTER          'f'             ///< Filter which CAN message types to allow
#define CMD_FILTER_NONE     '0'             ///< Allow all CAN message types
#define CMD_FILTER_T5       '5'             ///< Allow only Trionic 5 CAN message types
#define CMD_FILTER_T7       '7'             ///< Allow only Trionic 5 CAN message types
#define CMD_FILTER_T8       '8'             ///< Allow only Trionic 5 CAN message types

#define CMD_ACCEPT_CODE     'M'             ///< Mxxxxxxxx: Acceptance code e.g. 0x00000000 } accept
#define CMD_ACCEPT_MASK     'm'             ///< mxxxxxxxx: Acceptance mask e.g. 0xffffffff } all

#define CMD_VERSION         'V'             ///< Replies with Firmware and hardware version numbers; 2 bytes each
#define CMD_SERIAL_NUMBER   'N'             ///< Replies with serial number; 4 bytes

#define CMD_TIMESTAMP       'Z'             ///< Zn: n=0 means timestamp off, n=1 means timestamp is on
///< Replies a value in milliseconds with two bytes 0x0..0xfa5f
///< equivalent to 0..60 seconds



// static variables
static char cmd_buffer[CMD_BUF_LENGTH];     ///< command string buffer
static uint32_t can_id;                     ///< can message id
static uint32_t can_len;                    ///< can message length
static uint8_t can_msg[8];                  ///< can message frame - up to 8 bytes

// private functions
uint8_t execute_can_cmd();

// command argument macros
#define CHECK_ARGLENGTH(len) \
    if (cmd_length != len + 2) \
        return TERM_ERR

#define GET_NUMBER(target, offset, len)    \
    if (!ascii2int(target, cmd_buffer + 2 + offset, len)) \
        return TERM_ERR



void can232() {

    // main loop
    *cmd_buffer = '\0';
    char ret;
    char rx_char;
    while (true) {
        // read chars from USB
        if (pc.readable()) {
            // turn Error LED off for next command
            led4 = 0;
            rx_char = pc.getc();
            switch (rx_char) {
                    // 'ESC' key to go back to mbed Just4Trionic 'home' menu
                case '\e':
                    can_close();
                    can.attach(NULL);
                    return;
                    // end-of-command reached
                case TERM_OK :
                    // execute command and return flag via USB
                    ret = execute_can_cmd();
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

uint8_t execute_can_cmd() {
    uint8_t cmd_length = strlen(cmd_buffer);
    char cmd = *(cmd_buffer + 1);

    // command groups
    switch (*cmd_buffer) {
            // adapter commands
        case CMD_SEND_11BIT:
            if (cmd_length < 7) return TERM_ERR;
            GET_NUMBER(&can_id, -1, 3);
            GET_NUMBER(&can_len, 2, 1);
            if (cmd_length < (4 + (can_len * 2))) return TERM_ERR;
            for (uint8_t i = 0; i < (uint8_t)can_len; i++) {
                uint32_t result;
                GET_NUMBER(&result, (3 + (i * 2)), 2);
                can_msg[i] = (uint8_t)result;
            }
            return (can_send_timeout (can_id, (char*)can_msg, (uint8_t)can_len, 500)) ? TERM_OK : TERM_ERR;

        case CMD_SEND_29BIT:
            break;

        case CMD_CLOSE:
            can_close();
            return TERM_OK;
        case CMD_OPEN:
            can_open();
            return TERM_OK;

        case CMD_PRESET_SPEED:
            CHECK_ARGLENGTH(0);
//            can.attach(NULL);
           switch (cmd) {
                case CMD_SPEED_0:
                    can_configure(2, 10000, 0);
                    break;
                case CMD_SPEED_1:
                    can_configure(2, 20000, 0);
                    break;
                case CMD_SPEED_2:
                    can_configure(2, 50000, 0);
                    break;
                case CMD_SPEED_3:
                    can_configure(2, 100000, 0);
                    break;
                case CMD_SPEED_4:
                    can_configure(2, 125000, 0);
                    break;
                case CMD_SPEED_5:
                    can_configure(2, 250000, 0);
                    break;
                case CMD_SPEED_6:
                    can_configure(2, 500000, 0);
                    break;
                case CMD_SPEED_7:
                    can_configure(2, 800000, 0);
                    break;
                case CMD_SPEED_8:
                    can_configure(2, 1000000, 0);
                    break;
                default:
                    return TERM_ERR;
            }
            can.attach(&show_can_message);
            return TERM_OK;

        case CMD_DIRECT_SPEED:
            CHECK_ARGLENGTH(0);
            switch (cmd) {
                case CMD_SPEED_0:
                    can_configure(2, 47619, 0);
                    break;
                case CMD_SPEED_1:
                    can_configure(2, 500000, 0);
                    break;
                case CMD_SPEED_2:
                    can_configure(2, 615000, 0);
                    break;
                default:
                    return TERM_ERR;
            }
            can.attach(&show_can_message);
            return TERM_OK;

        case CMD_FILTER:
            CHECK_ARGLENGTH(0);
            switch (cmd) {
                case CMD_FILTER_NONE:
                    can_use_filters(FALSE);                             // Accept all messages (Acceptance Filters disabled)
                    return TERM_OK;
                case CMD_FILTER_T5:
                    can_reset_filters();
                    can_add_filter(2, 0x005);         //005h -
                    can_add_filter(2, 0x006);         //006h -
                    can_add_filter(2, 0x00C);         //00Ch -
                    can_add_filter(2, 0x008);         //008h -
                    return TERM_OK;
                case CMD_FILTER_T7:
                    can_reset_filters();
                    can_add_filter(2, 0x220);         //220h
                    can_add_filter(2, 0x238);         //238h
                    can_add_filter(2, 0x240);         //240h
                    can_add_filter(2, 0x258);         //258h
                    can_add_filter(2, 0x266);         //266h - Ack
                    return TERM_OK;
                    can_add_filter(2, 0x1A0);         //1A0h - Engine information
                    can_add_filter(2, 0x280);         //280h - Pedals, reverse gear
                    can_add_filter(2, 0x290);         //290h - Steering wheel and SID buttons
                    can_add_filter(2, 0x2F0);         //2F0h - Vehicle speed
                    can_add_filter(2, 0x320);         //320h - Doors, central locking and seat belts
                    can_add_filter(2, 0x370);         //370h - Mileage
                    can_add_filter(2, 0x3A0);         //3A0h - Vehicle speed
                    can_add_filter(2, 0x3B0);         //3B0h - Head lights
                    can_add_filter(2, 0x3E0);         //3E0h - Automatic Gearbox
                    can_add_filter(2, 0x410);         //410h - Light dimmer and light sensor
                    can_add_filter(2, 0x430);         //430h - SID beep request (interesting for Knock indicator?)
                    can_add_filter(2, 0x460);         //460h - Engine rpm and speed
                    can_add_filter(2, 0x4A0);         //4A0h - Steering wheel, Vehicle Identification Number
                    can_add_filter(2, 0x520);         //520h - ACC, inside temperature
                    can_add_filter(2, 0x530);         //530h - ACC
                    can_add_filter(2, 0x5C0);         //5C0h - Coolant temperature, air pressure
                    can_add_filter(2, 0x630);         //630h - Fuel usage
                    can_add_filter(2, 0x640);         //640h - Mileage
                    can_add_filter(2, 0x7A0);         //7A0h - Outside temperature
                    return TERM_OK;
                case CMD_FILTER_T8:
                    can_reset_filters();
                    can_add_filter(2, 0x645);         //645h - CIM
                    can_add_filter(2, 0x7E0);         //7E0h -
                    can_add_filter(2, 0x7E8);         //7E8h -
                    can_add_filter(2, 0x311);         //311h -
                    can_add_filter(2, 0x5E8);         //5E8h -
                    //can_add_filter(2, 0x101);         //101h -
                    return TERM_OK;
            }
            break;
    }

    // unknown command
    return TERM_ERR;
}

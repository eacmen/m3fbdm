
// t5utils.h - information and definitions needed for communicating with the T5 ECU

// (C) 2010, Sophie Dexter

#ifndef __T5UTILS_H__
#define __T5UTILS_H__

#include "mbed.h"

#include "common.h"
#include "canutils.h"

#define CMNDID 0x05
#define ACK_ID 0x06
#define RESPID 0x0C
#define RSETID 0x08

#define T5_RDCMND {0xC4,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}     // C4 Command template for reading something from the T5 ECU
#define T5_WRCMND {0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C4 Command template for writing something to the T5 ECU
#define T5ACKCMND {0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C6 Command for acknowledging receipt of last message from T5 ECU or
                                                                // asking it to send some more data (e.g. symbol table) I'm not sure which :lol:
#define T5_BTCMND {0xA5,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // A5 Command template for sending a bootloader to the T5 ECU
#define T5JMPCMND {0xC1,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C1 Command template for starting a bootloader that has been sent to the T5 ECU
#define T5EOFCMND {0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C3 Command for reading back the last address of FLASH in the T5 ECU
#define T5A_DCMND {0xC5,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C5 Command for setting analogue to digital registers (don't know what this is for)
#define T5RAMCMND {0xC7,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C7 Command for reading T5 RAM (all the symbols and adaption data is in RAM)


#define T5ERACMND {0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C0 Command for erasing T5 FLASH Chips !!! (Bootloader must be loaded before using)
#define T5RSTCMND {0xC2,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C2 Command for exiting the Bootloader and restarting the T5 ECU (Bootloader must be loaded before using)
#define T5SUMCMND {0xC8,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C8 Command for reading T5 FLASH Checksum (Bootloader must be loaded before using)
#define T5TYPCMND {0xC9,0x00,0x00,0x00,0x00,0x00,0x00,0x00}     // C9 Command for reading T5 FLASH Chip types (Bootloader must be loaded before using)

#define T5STARTMSG {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00}    // T5 sends this message whenever the ignition is turned on? - message id is RESPID
#define T5RESETMSG {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}    // T5 sends this message to say that it has reset - message id is RSETID

#define T5MESSAGETIMEOUT 500            // 500 milliseconds (half a second) - Seems to be plenty of time to wait for messages on the CAN bus
#define T5CHECKSUMTIMEOUT 2000          // 2 seconds (2,000 milliseconds) - Usually takes less than a second so allowing 2 is plenty
#define T5ERASETIMEOUT 40000            // 40 seconds (60,000 milliseconds) - Usually takes less than 20 seconds so allowing 40 is plenty

extern bool T5Open();
extern bool T5Close();
extern char T5WaitResponse();
extern bool T5WaitResponsePrint();
extern char *T5GetSymbol(char *a_symbol);
extern bool T5ReadCmnd(char c);
extern bool T5WriteCmnd(uint32_t addr, char data);
extern bool t5_can_read_data(char *data, uint32_t addr);
extern bool T5Ack();
extern bool t5_can_send_boot_address(uint32_t addr, uint8_t len);
extern bool t5_can_send_boot_frame(char *T5TxMsg);
extern bool T5StartBootLoader (uint32_t addr);

extern bool t5_boot_checksum_command(uint32_t* result);
extern bool t5_boot_reset_command();
extern bool t5_boot_get_flash_type(uint32_t* result, uint8_t* make, uint8_t* type);
extern bool t5_boot_erase_command();
extern bool t5_boot_c3_command(uint32_t* result, uint16_t* mode);

#endif
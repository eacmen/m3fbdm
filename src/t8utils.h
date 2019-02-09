
// t8utils.h - information and definitions needed for communicating with the T7 ECU

// (C) 2011, 2012 Sophie Dexter

#ifndef __T8UTILS_H__
#define __T8UTILS_H__

#include "mbed.h"

#include "common.h"
#include "canutils.h"

//#include "t8bootloaders.h"
#include "gmlan.h"


#define T8TSTRID 0x7E0
#define T8ECU_ID 0x7E8
#define T8ANYMSG 0x0

// initialise T8

//#define T8REQVIN    {0x02,0x09,0x02,0x00,0x00,0x00,0x00,0x00}
// Request VIN using ReadDataByIdentifier method using DID
#define T8REQVIN    {0x02,0x1A,0x90,0x00,0x00,0x00,0x00,0x00}


// A "Flow Control" message. Send to let T8 it is OK to send the rest of the messages it has
#define T8FLOCTL    {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00}


// read_trionic8


// flash_trionic8


#define T8MESSAGETIMEOUT 50             // 50 milliseconds (0.05 of a second) - Seems to be plenty of time to wait for messages on the CAN bus
#define T8LONGERTIMEOUT 500             // 500 milliseconds (0.5 of a second) - Some messages seem to need longer
#define T8CHECKSUMTIMEOUT 2000          // 2 seconds (2,000 milliseconds) - Usually takes less than a second so allowing 2 is plenty
#define T8CONNECTTIMEOUT 5000           // 5 seconds (5,000 milliseconds) - Usually takes 3 seconds so allowing 5 is plenty
#define T8ERASETIMEOUT 120000            // 120 seconds (120,000 milliseconds) - Usually takes less than 90 seconds so allowing 120 is plenty

extern bool t8_initialise();
extern bool t8_show_VIN();
extern bool t8_write_VIN();
extern bool t8_authenticate(uint32_t ReqID, uint32_t RespID, char level);
extern bool t8_dump();
extern bool t8_flash();
extern bool t8_recover();


#endif
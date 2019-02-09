/*******************************************************************************

common.h - information and definitions needed by parts of Just4Trionic
(c) by 2010 Sophie Dexter
portions (c) 2009, 2010 by Janis Silins (johnc)

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

//#include <stdint.h>
//#include <stdbool.h>

#undef DEBUG
#ifndef DEBUG
//#define DEBUG 1
//#define DEBUG
#endif

#include "mbed.h"

#include "sizedefs.h"
#include "strings.h"
#include "interfaces.h"
#include "t8bootloaders.h"
#include "t5bootloaders.h"

// build configuration
//#define IGNORE_VCC_PIN            ///< uncomment to ignore the VCC pin

// constants
#define FW_VERSION_MAJOR    0x1     ///< firmware version
#define FW_VERSION_MINOR    0x6        

#define CR 0x0D
#define NL 0x0A
#define BELL 0x07

#define TRUE 1
#define FALSE 0


// bit macros
#define SETBIT(x,y)         (x |= (y))                ///< set bit y in byte x
#define CLEARBIT(x,y)       (x &= (~(y)))            ///< clear bit y in byte x
#define CHECKBIT(x,y)       (((x) & (y)) == (y))    ///< check bit y in byte x

// command return flags and character constants
#define TERM_OK             13            ///< command terminator or success flag
#define TERM_ERR            7            ///< error flag
#define TERM_BREAK          0x1b        ///< command break flag

#define ERR_COUNT           255            ///< maximum error cycles

#define FILE_BUF_LENGTH      0x1000              ///< file buffer size
static char file_buffer[FILE_BUF_LENGTH];     ///< file buffer

static const uint8_t T8BootloaderRead[] = T8_BOOTLOADER_DUMP;
static const uint8_t T8BootLoaderWrite[] = T8_BOOTLOADER_PROG;

static const uint8_t T5BootLoader[] = MYBOOTY;

// FLASH chip manufacturer id values
#define AMD                 0x01
#define CSI                 0x31
#define INTEL               0x89
#define ATMEL               0x1F
#define SST                 0xBF
#define ST                  0x20
#define AMIC                0x37

// FLASH chip type values
#define INTEL28F512         0xB8
#define AMD28F512           0x25
#define INTEL28F010         0xB4        // and CSI
#define AMD28F010           0xA7
#define AMD29F010           0x20        // and ST
#define ATMEL29C512         0x5D
#define ATMEL29C010         0xD5
#define SST39SF010          0xB5
#define ST29F010            0x20
#define AMICA29010L         0xA4
#define AMD29F400T          0x23
#define AMD29F400B          0xAB        // !!! NOT USED IN T7 !!!
//#define 29F400T             0x2223
//#define 29F400B             0x22AB
#define AMD29BL802C         0x81
//#define AMD29BL802C         0x2281


// TRIONIC ECU Start addresses
#define T52FLASHSTART       0x60000
#define T55FLASHSTART       0x40000
#define T7FLASHSTART        0x00000
#define T8FLASHSTART        0x00000

// TRIONIC ECU FLASH sizes
#define T52FLASHSIZE        0x20000
#define T55FLASHSIZE        0x40000
#define T7FLASHSIZE         0x80000
#define T8FLASHSIZE         0x100000

// TRIONIC ECU Last address
#define TRIONICLASTADDR     0x7FFFF

// TRIONIC ECU RAM sizes
#define T5RAMSIZE           0x8000
#define T7RAMSIZE           0x8000

// Initial Stack pointer values used by Trionic (1st 4 bytes of BIN file)
#define T5POINTER           0xFFFFF7FC
#define T7POINTER           0xFFFFEFFC
#define T8POINTER           0x00100C00

// public functions
void led_on(uint8_t led);
bool ascii2int(uint32_t* val, const char* str, uint8_t length);

#endif              // __COMMON_H__

//-----------------------------------------------------------------------------
//    EOF
//-----------------------------------------------------------------------------

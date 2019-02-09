/*******************************************************************************

bdmdriver.cpp
(c) 2014 by Sophie Dexter

BD32 like 'syscall' processor for BDM resident driver functions

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "bdmdriver.h"

FILE *fp = NULL;

//private functions
bool bdmSyscallPuts (void);
bool bdmSyscallPutchar(void);
bool bdmSyscallGets(void);
bool bdmSyscallGetchar(void);
bool bdmSyscallGetstat(void);
bool bdmSyscallFopen(void);
bool bdmSyscallFclose(void);
bool bdmSyscallFread(void);
bool bdmSyscallFwrite(void);
bool bdmSyscallFtell(void);
bool bdmSyscallFseek(void);
bool bdmSyscallFgets(void);
bool bdmSyscallFputs(void);
bool bdmSyscallEval(void);
bool bdmSyscallFreadsrec(void);

//-----------------------------------------------------------------------------
/**
Loads the contenst of a uint8_t array into the target's memory starting at the
specified address

@param        dataArray[]       uint8_t array conataining data for BDM tartget
@param        startAddress      Start address to load BDM memory to

@return                    succ / fail
*/
bool bdmLoadMemory(uint8_t dataArray[], uint32_t startAddress, uint32_t dataArraySize)
{
//    for (uint32_t i = 0; i < sizeof(residentDriver); i++) {
//        if(memwrite_byte(&driverAddress, residentDriver[i]) != TERM_OK) return false;
//        driverAddress++;
//    }
    uint32_t driverSize = dataArraySize;
    uint32_t driverOffset = 0;
    uint32_t driverLong = 0;
    uint16_t driverWord = 0;
    uint8_t driverByte = 0;
    // Check that there is something to send
    if (driverSize == 0) return false;
    // Send the first 1-4 bytes as efficiently as possible over BDM
    switch (driverSize % 4) {
        case 3:
            driverWord = (dataArray[driverOffset++] << 8) | dataArray[driverOffset++];
            if(memwrite_word(&startAddress, driverWord) != TERM_OK) return false;
            driverByte = dataArray[driverOffset++];
            if(memfill_byte(driverByte) != TERM_OK) return false;
            break;
        case 2:
            driverWord = (dataArray[driverOffset++] << 8) | dataArray[driverOffset++];
            if(memwrite_word(&startAddress, driverWord) != TERM_OK) return false;
            break;
        case 1:
            driverByte = dataArray[driverOffset++];
            if(memwrite_byte(&startAddress, driverByte) != TERM_OK) return false;
            break;
        case 0:
            for (uint32_t i = 0; i < 4; i++) {
                driverLong <<= 8;
                driverLong |= dataArray[driverOffset++];
            }
            if(memwrite_long(&startAddress, &driverLong) != TERM_OK) return false;
            break;
//        default: // There shouldn't be a default case
    }
    // transfer the rest as 'longs' to make best use of BDM transfer speed
//    printf("driverOffset 0x%08x, driverSize 0x%08x\r\n", driverOffset, driverSize);
    while (driverOffset < driverSize) {
        for (uint32_t i = 0; i < 4; i++) {
            driverLong <<= 8;
            driverLong |= dataArray[driverOffset++];
        }
        if(memfill_long(&driverLong) != TERM_OK) return false;
    }
//    printf("driverOffset 0x%08x, driverSize 0x%08x\r\n", driverOffset, driverSize);
    return true;
}

//-----------------------------------------------------------------------------
/**
Starts a BDM resident driver at the current PC (Program Counter) or a specified
if given. The driver is allowed to run for a maximum period, maxtime, specified
as milliseconds.

@param        addr        BDM driver address (0 to continue from current PC)
@param        maxtime     how long to allow driver to execute (milliseconds)

@return                    succ / fail
*/
bool bdmRunDriver(uint32_t addr, uint32_t maxtime)
{
    // Start BDM driver and allow it up to 200 milliseconds to update 256 Bytes
    // Upto 25 pulses per byte, 16us per pulse, 256 Bytes
    // 25 * 16 * 256 = 102,400us plus overhead for driver code execution time
    // Allowing up to 200 milliseconds seems like a good allowance.
    if (run_chip(&addr) != TERM_OK) {
        printf("Failed to start BDM driver.\r\n");
        return false;
    }
    timeout.reset();
    timeout.start();
    // T5 ECUs' BDM interface seem to have problems when the running the CPU and
    // sometimes shows the CPU briefly switching between showing BDM mode or that
    // the CPU is running.
    // I 'debounce' the interface state to workaround this erratic bahaviour
    for (uint32_t debounce = 0; debounce < 5; debounce++) {
        while (IS_RUNNING) {
            debounce = 0;
            if (timeout.read_ms() > maxtime) {
                printf("Driver did not return to BDM mode.\r\n");
                timeout.stop();
                return false;
            }
        }
        wait_us(1);
    }
    timeout.stop();
    return true;
}


//-----------------------------------------------------------------------------
/**
Starts a BDM resident driver at the current PC (Program Counter) or a specified
if given. The driver is allowed to run for a maximum period, maxtime, specified
as milliseconds.

@param        addr        BDM driver address (0 to continue from current PC)
@param        maxtime     how long to allow driver to execute (milliseconds)

@return                    DONE, CONTINUE, ERROR

*/

uint8_t bdmProcessSyscall(void)
{

    uint32_t syscall = 0xFFFFFFFF;
    if (adreg_read(&syscall, 0x0) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return ERROR;
    }
    syscall &= 0xFF;
//    printf("SYSCALL 0x%08x\r\n", syscall);
    switch (syscall) {
        case QUIT:
            if (adreg_read(&syscall, 0x1) != TERM_OK) {
                printf("Failed to read BDM register.\r\n");
                return ERROR;
            }
            return DONE;
        case PUTS:
            if (!bdmSyscallPuts()) return ERROR;
            break;
        case PUTCHAR:
            if (!bdmSyscallPutchar()) return ERROR;
            break;
        case GETS:
            if (!bdmSyscallGets()) return ERROR;
            break;
        case GETCHAR:
            if (!bdmSyscallGetchar()) return ERROR;
            break;
        case GETSTAT:
            if (!bdmSyscallGetstat()) return ERROR;
            break;
        case FOPEN:
            if (!bdmSyscallFopen()) return ERROR;
            break;
        case FCLOSE:
            if (!bdmSyscallFclose()) return ERROR;
            break;
        case FREAD:
            if (!bdmSyscallFread()) return ERROR;
            break;
        case FWRITE:
            if (!bdmSyscallFwrite()) return ERROR;
            break;
        case FTELL:
            if (!bdmSyscallFtell()) return ERROR;
            break;
        case FSEEK:
            if (!bdmSyscallFseek()) return ERROR;
            break;
        case FGETS:
            if (!bdmSyscallFgets()) return ERROR;
            break;
        case FPUTS:
            if (!bdmSyscallFputs()) return ERROR;
            break;
        case EVAL:
            if (!bdmSyscallEval()) return ERROR;
            break;
        case FREADSREC:
            if (!bdmSyscallFreadsrec()) return ERROR;
            break;
        default:
            printf("!!! Unknown BDM Syscall !!!\r\n");
            return ERROR;
    }
    return CONTINUE;
}


bool bdmSyscallPuts()
{
    uint32_t bdm_string_address = 0, bdm_return = 0;
    if (adreg_read(&bdm_string_address, 0x8) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
// a loop to read chars from BDM into a string
    char bdm_string[256];
    for (uint32_t i = 0; i < sizeof(bdm_string); i++) {
        bdm_string[i] = 0x0;
    }
    uint32_t i = 0;
    do {
        if (memread_byte((uint8_t*)(bdm_string+i), &bdm_string_address) != TERM_OK) {
            printf("Failed to read BDM memory at address 0x%08x.\r\n", bdm_string_address);
            return false;
        }
        bdm_string_address++;
    } while ( (bdm_string[i++] != 0x0) && (i < sizeof(bdm_string)) );
// print the string to stdout (USB virtual serial port)
    printf(bdm_string);
// Send BDM return code in D0 (always 0x0 for PUTS)
    if (adreg_write(0x0, &bdm_return) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallPutchar()
{
    uint32_t bdm_character = 0, bdm_return = 0;
    // read char from BDM
    if (adreg_read(&bdm_character, 0x1) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    // print the char to USB virtual serial port
    pc.putc((char)bdm_character);
    // Send BDM return code in D0 (always 0x0 for PUTCHAR)
    if (adreg_write(0x0, &bdm_return) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallGets()
{
    printf("BDM GETS Syscall not supported.\r\n");
    return ERROR;
}

bool bdmSyscallGetchar()
{
    // get a char from the USB virtual serial port
    uint32_t bdm_return = (uint32_t)pc.getc();
    // Send the char to BDM in D0
    if (adreg_write(0x0, &bdm_return) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallGetstat(void)
{
    printf("BDM GETSTAT Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallFopen(void)
{
    uint32_t bdm_filename_address = 0, bdm_filemode_address = 0;
    if (adreg_read(&bdm_filename_address, 0x8) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    if (adreg_read(&bdm_filemode_address, 0x9) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    // a loop to initialise the strings to 0x0
    char filename_string[80], filemode_string[5];
    uint32_t i = 0;
    for (i = 0; i < sizeof(filename_string); i++) {
        filename_string[i] = 0x0;
    }
    for (i = 0; i < sizeof(filemode_string); i++) {
        filemode_string[i] = 0x0;
    }
    i = 0;
    // a loop to read chars from BDM into a string
    do {
        if (memread_byte((uint8_t*)filename_string[i], &bdm_filename_address) != TERM_OK) {
            printf("Failed to read BDM memory at address 0x%08x.\r\n", bdm_filename_address);
            return false;
        }
        bdm_filename_address++;
    } while ( (filename_string[i++] != 0x0) && (i < sizeof(filename_string)) );
    do {
        if (memread_byte((uint8_t*)filemode_string[i], &bdm_filemode_address) != TERM_OK) {
            printf("Failed to read BDM memory at address 0x%08x.\r\n", bdm_filemode_address);
            return false;
        }
        bdm_filemode_address++;
    } while ( (filemode_string[i++] != 0x0) && (i < sizeof(filemode_string)) );
    // Open the file
    fp = fopen(filename_string, filemode_string);    // Open "modified.hex" on the local file system for reading
    // Send BDM return code in D0
    if (adreg_write(0x0, (uint32_t*)fp) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallFclose(void)
{
    uint32_t close_result = fclose(fp);
    // Send BDM return code in D0
    if (adreg_write(0x0, &close_result) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallFread(void)
{
    uint32_t bdm_byte_count, bdm_buffer_address, bdm_file_handle = NULL;
    if (adreg_read(&bdm_file_handle, 0x1) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    if (adreg_read(&bdm_byte_count, 0x2) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    if (adreg_read(&bdm_buffer_address, 0x9) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    uint32_t bytes_read = fread(&file_buffer[0],1,bdm_byte_count,fp);
    for (uint32_t byte_count = 0; byte_count < bytes_read; byte_count++) {
        if (byte_count == 0x0) {
            if(memwrite_byte(&bdm_buffer_address, file_buffer[byte_count]) != TERM_OK) return false;
        } else {
            if(memfill_byte(file_buffer[byte_count]) != TERM_OK) return false;
        }
    }
    if (adreg_write(0x0, &bytes_read) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallFwrite(void)
{
    printf("BDM FWRITE Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallFtell(void)
{
    printf("BDM FTELL Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallFseek(void)
{
    uint32_t bdm_byte_offset, bdm_file_origin, bdm_file_handle = NULL;
    if (adreg_read(&bdm_file_handle, 0x1) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    if (adreg_read(&bdm_byte_offset, 0x2) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    if (adreg_read(&bdm_file_origin, 0x3) != TERM_OK) {
        printf("Failed to read BDM register.\r\n");
        return false;
    }
    uint32_t origin;
    switch (bdm_file_origin) {
        case 0x2:
            origin = SEEK_END;
            break;
        case 0x1:
            origin = SEEK_CUR;
            break;
        case 0x0:
        default:
            origin = SEEK_SET;
            break;
    }
    uint32_t fseek_result = fseek ( fp ,bdm_byte_offset ,origin );
    if (adreg_write(0x0, &fseek_result) != TERM_OK) {
        printf("Failed to write BDM register.\r\n");
        return false;
    }
    return true;
}

bool bdmSyscallFgets(void)
{
    printf("BDM FGETS Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallFputs(void)
{
    printf("BDM FPUTS Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallEval(void)
{
    printf("BDM EVAL Syscall not supported.\r\n");
    return false;
}

bool bdmSyscallFreadsrec(void)
{
    printf("BDM FREADSREC Syscall not supported.\r\n");
    return false;
}

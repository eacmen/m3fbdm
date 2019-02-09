/*******************************************************************************

t7utils.cpp
(c) 2011, 2012 by Sophie Dexter
portions (c) Tomi Liljemark (firstname.surname@gmail.com)

This C++ module provides functions for communicating simple messages to and from
the T7 ECU

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "t7utils.h"


//
// t7_initialise
//
// sends an initialisation message to the T7 ECU
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//


bool t7_initialise()
{
    // send a can message to the T7 requesting that it initialises CAN communication with Just4Trionic
    char T7TxMsg[] = T7INITMSG;
    if (!can_send_timeout (T7CMNDID, T7TxMsg, 8, T7MESSAGETIMEOUT))
        return FALSE;
    // wait for the T7 to reply
    char T7RxMsg[8];
    // if a message is not received, has the wrong id
    if (!can_wait_timeout(T7RESPID, T7RxMsg, 8, T7MESSAGETIMEOUT))
        return FALSE;
    /* DEBUG info...
        for (int i = 0; i < 8; i++ ) printf("0x%02X ", T7RxMsg[i] );
        printf(" init\r\n");
    */
    return TRUE;
}

//
// t7_authenticate
//
// sends an authentication message to the T7 ECU
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//

bool t7_authenticate()
{
    uint16_t seed, key;
//    uint16_t i;
    char T7TxAck[] = T7ACK_MSG;
    char T7TxMsg[] = T7SEC_MSG;
    char T7TxKey[] = T7KEY_MSG;
    char T7RxMsg[8];
    // Send "Request Seed" to Trionic7
    if (!can_send_timeout (T7SEC_ID, T7TxMsg, 8, T7MESSAGETIMEOUT))
        return FALSE;
    // wait for the T7 to reply
    // Read "Seed"
    // if a message is not received id return false
    if (!can_wait_timeout(T7SEC_RX, T7RxMsg, 8, T7MESSAGETIMEOUT))
        return FALSE;
    /* DEBUG info...
        for (i = 0; i < 8; i++ ) printf("0x%02X ", T7RxMsg[i] );
        printf(" seed\r\n");
    */
    // Send Ack
    T7TxAck[3] = T7RxMsg[0] & 0xBF;
    if (!can_send_timeout (T7ACK_ID, T7TxAck, 8, T7MESSAGETIMEOUT))
        return FALSE;
    // Send "Key", try two different methods of calculating the key
    seed = T7RxMsg[5] << 8 | T7RxMsg[6];
    for (int method = 0; method < 2; method++ ) {
        key = seed << 2;
        key &= 0xFFFF;
        key ^= ( method ? 0x4081 : 0x8142 );
        key -= ( method ? 0x1F6F : 0x2356 );
        key &= 0xFFFF;
        T7TxKey[5] = ( key >> 8 ) & 0xFF;
        T7TxKey[6] = key & 0xFF;
        if (!can_send_timeout (T7SEC_ID, T7TxKey, 8, T7MESSAGETIMEOUT))
            return FALSE;
        // Wait for response
        // if a message is not received id return false
        if (!can_wait_timeout(T7SEC_RX, T7RxMsg, 8, T7MESSAGETIMEOUT))
            return FALSE;
        /* DEBUG info...
                for (i = 0; i < 8; i++ ) printf("0x%02X ", T7RxMsg[i] );
                printf(" key %d 0x%02X 0x%02X\r\n", method, T7RxMsg[3], T7RxMsg[5]);
        */
        // Send Ack
        T7TxAck[3] = T7RxMsg[0] & 0xBF;
        if (!can_send_timeout (T7ACK_ID, T7TxAck, 8, T7MESSAGETIMEOUT)) {
            /* DEBUG info...
                        printf("Key ACK message timeout\r\n");
            */
            return FALSE;
        }
        if ( T7RxMsg[3] == 0x67 && T7RxMsg[5] == 0x34 ) {
            /* DEBUG info...
                        printf("Key %d Accepted\r\n", method);
            */
            return TRUE;
        } else {
            /* DEBUG info...
                        printf("Key %d Failed\r\n", method);
            */
        }
    }
    return FALSE;
}
//
// t7_dump
//
// dumps the T7 BIN File
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//

bool t7_dump(bool blockmode)
{
    uint32_t received;
    uint8_t byte_count, retries, i;
    char T7_dump_jumpa[] = T7DMPJP1A;
    char T7_dump_jumpb[] = T7DMPJP1B;
    char T7_dump_ack[] = T7DMP_ACK;
    char T7_dump_data[] = T7DMPDATA;
    char T7_dump_end[] = T7DMP_END;
    char T7RxMsg[8];

    printf("Creating FLASH dump file...\r\n");
    FILE *fp = fopen("/local/original.bin", "w");    // Open "original.bin" on the local file system for writing
    if (!fp) {
        perror ("The following error occured");
        return TERM_ERR;
    }

    timer.reset();
    timer.start();

    received = 0;
    printf("  0.00 %% complete.\r");
    while (received < T7FLASHSIZE) {
//        T7_dump_jumpa[7] = ((T7FLASHSIZE - received) < 0xEF) ? (T7FLASHSIZE - received) : 0xEF;
        T7_dump_jumpb[2] = (received >> 16) & 0xFF;
        T7_dump_jumpb[3] = (received >> 8) & 0xFF;
        T7_dump_jumpb[4] = received & 0xFF;
        // Send read address and length to Trionic
        if (!can_send_timeout (T7SEC_ID, T7_dump_jumpa, 8, T7MESSAGETIMEOUT)) {
            printf("err t7utils line: %d\r\n", __LINE__ );
            fclose(fp);
            return FALSE;
        }
        if (!can_send_timeout (T7SEC_ID, T7_dump_jumpb, 8, T7MESSAGETIMEOUT)) {
            printf("err t7utils line: %d\r\n", __LINE__ );
            fclose(fp);
            return FALSE;
        }
        // Wait for a response
        if (!can_wait_timeout(T7SEC_RX, T7RxMsg, 8, T7MESSAGETIMEOUT)) {
            printf("err t7utils line: %d\r\n", __LINE__ );
            fclose(fp);
            return FALSE;
        }
        /* DEBUG info...
            for (i = 0; i < 8; i++ ) printf("0x%02X ", T7RxMsg[i] );
            printf(" seed\r\n");
        */
        // Send Ack
        T7_dump_ack[3] = T7RxMsg[0] & 0xBF;
        if (!can_send_timeout (T7ACK_ID, T7_dump_ack, 8, T7MESSAGETIMEOUT)) {
            printf("ERROR Asking1: %5.1f %% done\r\n", 100*(float)received/(float)T7FLASHSIZE);
            printf("err t7utils line: %d\r\n", __LINE__ );
            fclose(fp);
            return FALSE;
        }
        if ((T7RxMsg[3] != 0x6C) ||(T7RxMsg[4] != 0xF0)) {
            printf("ERROR Asking2: %5.1f %% done\r\n", 100*(float)received/(float)T7FLASHSIZE);
            printf("err t7utils line: %d\r\n", __LINE__ );
            fclose(fp);
            return FALSE;
        }
        // Ask T7 ECU to start sending data
        for (retries = 0 ; retries <10 ; retries++ ) {
            if (!can_send_timeout (T7SEC_ID, T7_dump_data, 8, T7MESSAGETIMEOUT)) {
                printf("err t7utils line: %d\r\n", __LINE__ );
                fclose(fp);
                return FALSE;
            }
            // Read mesages from the T7 ECU
            byte_count = 0;
            T7RxMsg[0] = 0x00;
            while (T7RxMsg[0] != 0x80 && T7RxMsg[0] != 0xC0) {
                if (!can_wait_timeout(T7SEC_RX, T7RxMsg, 8, T7MESSAGETIMEOUT))
                    break;
                // Need to process the received data here!
                // Send Ack
                T7_dump_ack[3] = T7RxMsg[0] & 0xBF;
                if (!can_send_timeout (T7ACK_ID, T7_dump_ack, 8, T7MESSAGETIMEOUT)) {
                    printf("ERROR processing: %5.1f %% done\r\n", 100*(float)received/(float)T7FLASHSIZE);
                    printf("err t7utils line: %d\r\n", __LINE__ );
                    fclose(fp);
                    return FALSE;
                }
//                /* DEBUG info...
//                for (i = 0; i < 8; i++ ) printf("0x%02X ", T7RxMsg[i] );
//                for (i = 2; i < 8; i++ ) printf("%c ", T7RxMsg[i] );
//                printf(" data\r\n");
                for (i = 2; i < 8; i++ )
                    file_buffer[byte_count++] = (T7RxMsg[i]);
//                */
            }
            // Success if these conditions met
            if (T7RxMsg[0] == 0x80 || T7RxMsg[0] == 0xC0)
                break;
//            printf("retries: %d\r\n", retries);
        }
        if (retries > 9) {
            printf("err t7utils line: %d\r\n", __LINE__ );
            printf("Retries: %d, Done: %5.2f %%\r\n", retries, 100*(float)received/(float)T7FLASHSIZE );
            fclose(fp);
            return FALSE;
        }
//        received += 0xEF;
        received += 0x80;
//        printf("Retries: %d, Done: %5.2f %%\r\n", retries, 100*(float)received/(float)T7FLASHSIZE );
        printf("%6.2f\r", 100*(float)received/(float)T7FLASHSIZE );
        fwrite((file_buffer + 3), 1, 0x80, fp);
        if (ferror (fp)) {
            fclose (fp);
            printf ("Error writing to the FLASH BIN file.\r\n");
            return TERM_ERR;
        }
    }
    printf("\n");
    // Send Message to T7 ECU to say that we have finished
    if (!can_send_timeout (T7SEC_ID, T7_dump_end, 8, T7MESSAGETIMEOUT)) {
        fclose(fp);
        return FALSE;
    }
    // Wait for response
    if (!can_wait_timeout(T7SEC_RX, T7RxMsg, 8, T7MESSAGETIMEOUT)) {
        fclose(fp);
        return FALSE;
    }
// Send Ack
    T7_dump_ack[3] = T7RxMsg[0] & 0xBF;
    if (!can_send_timeout (T7ACK_ID, T7_dump_ack, 8, T7MESSAGETIMEOUT)) {
        printf("ERROR closing1: %5.1f %% done\r\n", 100*(float)received/(float)T7FLASHSIZE);
        printf("err t7utils line: %d\r\n", __LINE__ );
        fclose(fp);
        return FALSE;
    }
    if (T7RxMsg[3] != 0xC2) {
        printf("ERROR closing2: %5.1f %% done\r\n", 100*(float)received/(float)T7FLASHSIZE);
        printf("err t7utils line: %d\r\n", __LINE__ );
        fclose(fp);
        return FALSE;
    }
    timer.stop();
    printf("SUCCESS! Getting the FLASH dump took %#.1f seconds.\r\n",timer.read());
    fclose(fp);
    return TRUE;
}

bool t7_erase()
{
    char T7_erase_msga[]    = { 0x40, 0xA1, 0x02, 0x31, 0x52, 0x00, 0x00, 0x00 };
    char T7_erase_msgb[]    = { 0x40, 0xA1, 0x02, 0x31, 0x53, 0x00, 0x00, 0x00 };
    char T7_erase_confirm[] = { 0x40, 0xA1, 0x01, 0x3E, 0x00, 0x00, 0x00, 0x00 };
    char T7_erase_ack[]     = { 0x40, 0xA1, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00 };
    char data[8];
    int i;

    printf("Erasing T7 ECU FLASH...\r\n");

    data[3] = 0;
    i = 0;
    while ( data[3] != 0x71 && i < 10) {
        // Send "Request to ERASE" to Trionic
        if (!can_send_timeout (T7SEC_ID, T7_erase_msga, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        T7_erase_ack[3] = data[0] & 0xBF;
        if (!can_send_timeout (T7ACK_ID, T7_erase_ack, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        wait_ms(100);
        i++;
        printf(".");
    }
    printf("\r\n");
    // Check to see if erase operation lasted longer than 1 sec...
    if (i >=10) {
        printf("Second Message took too long'\r\n");
        return FALSE;
    }
    data[3] = 0;
    i = 0;
    while ( data[3] != 0x71 && i < 200) {
        // Send "Request to ERASE" to Trionic
        if (!can_send_timeout (T7SEC_ID, T7_erase_msgb, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        T7_erase_ack[3] = data[0] & 0xBF;
        if (!can_send_timeout (T7ACK_ID, T7_erase_ack, 8, T7MESSAGETIMEOUT)) {
            printf("err %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        wait_ms(100);
        i++;
        printf(".");
    }
    printf("\r\n");
    // Check to see if erase operation lasted longer than 20 sec...
    if (i >=200) {
        printf("Second Message took too long'\r\n");
        return FALSE;
    }

    // Confirm erase was successful?
    // (Note: no acknowledgements used for some reason)
    if (!can_send_timeout (T7SEC_ID, T7_erase_confirm, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if ( data[3] != 0x7E ) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    wait_ms(100);
    if (!can_send_timeout (T7SEC_ID, T7_erase_confirm, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if ( data[3] != 0x7E ) {
        printf("FAILURE: Unable to erase the FLASH .\r\n");
        return FALSE;
    }
    printf("SUCCESS: The FLASH has been erased.\r\n");
    return TRUE;
}


/// Open and check the bin file to make sure that it is a valid T7 BIN file
///
/// params  full filename including path of the T7 BIN file
///
/// returns a pointer to the T7 BIN file
///         or a null pointer if it is invalid in some way

FILE * t7_file_open(const char* fname)
{
    printf("Checking the FLASH BIN file...\r\n");
    FILE *fp = fopen(fname, "r");    // Open "modified.bin" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the BIN file MODIFIED.BIN\r\n");;
        return NULL;
    }
    // obtain file size - it should match the size of the FLASH chips:
    fseek (fp , 0 , SEEK_END);
    uint32_t file_size = ftell (fp);
    rewind (fp);
    if (file_size != T7FLASHSIZE) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T7 ECU :-(\r\n");
        printf("T7's FLASH chip size is: %#010x bytes.\r\n", T7FLASHSIZE);
        printf("The BIN's file size is: %#010x bytes.\r\n", file_size);
        return NULL;
    }

    // read the initial stack pointer value in the BIN file - it should match the value expected for the type of ECU
    uint8_t stack_byte = 0;
    uint32_t stack_long = 0;
    for (uint32_t i=0; i<4; i++) {
        if (!fread(&stack_byte,1,1,fp)) {
            printf("Error reading the BIN file MODIFIED.BIN\r\n");
            return NULL;
        }
        stack_long <<= 8;
        stack_long |= stack_byte;
    }
    rewind (fp);
    if (stack_long != T7POINTER) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T7 ECU :-(\r\n");
        printf("A T7 BIN file should start with: %#010x.\r\n", T7POINTER);
        printf("This file starts with : %#010x.\r\n", stack_long);
        return NULL;
    }
    printf("The BIN file appears to OK :-)\r\n");
    return fp;
}


bool t7_flash_segment(FILE *fp, uint32_t address, uint32_t size, bool blockmode)
{
    char T7_flash_start[]   = T7FLASTRT;
    char T7_flash_size[]    = T7FLASIZE;
    char T7_flash_ack[]     = T7FLA_ACK;
    char data[8];

    int32_t i, k;

    uint32_t current_address = address;
    uint32_t segment_end = address + size;

    if ((current_address > T7FLASHSIZE) || (segment_end > T7FLASHSIZE)) {
        printf("Attempting to FLASH outside of FLASH region\r\n");
        printf("Start Address: %#010x, End Address: %#010x.\r\n", current_address, segment_end);
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }

    // blocksize can be any size between 1 and 250 (0x01 - 0xFA)
    // Better performance is realised with a bigger blocksize
    //
    // The following convenient sizes to use for testing
    //     0x04, 0x10 and 0x40
    // A value of 0x70 is useful when programming the BIN region
    //uint32_t blocksize = (blockmode == true) ? 0x70 : 0x04;
    uint32_t blocksize = (blockmode == true) ? 0xFA : 0x04;
    // Sanity check 
    if (blocksize < 0x01) blocksize = 0x01;
    if (blocksize > 0xFA) blocksize = 0xFA;
    uint32_t blockquantity, blockremainder, blockfirst, blockduring;

    if (fseek (fp , address , SEEK_SET) != 0x0) {
        printf("Cannot move to file position %#010x.\r\n", current_address);
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    for(i=0; i<3; i++) {
        T7_flash_start[6-i] = (address >> (8*i)) & 0xff;
        T7_flash_size[4-i] = (size >> (8*i)) & 0xff;
    }

    // Send "Request Download - tool to module" to Trionic for the BIN file code and 'cal' data
    if (!can_send_timeout (T7SEC_ID, T7_flash_start, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_send_timeout (T7SEC_ID, T7_flash_size, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    T7_flash_ack[3] = data[0] & 0xBF;
    if (!can_send_timeout (T7ACK_ID, T7_flash_ack, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if ( data[3] != 0x74 ) {
        printf("Cannot Update FLASH, message refused.\r\n");
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }

// FLASH main Binary image up to address + size
    printf("  0.00 %% complete.\r");
    while (current_address < segment_end) {
        if ((segment_end - current_address) < blocksize) {
            blocksize = (segment_end - current_address);
            wait_ms(T7MESSAGETIMEOUT);
        }
        if (blocksize > 4) {
            blockfirst = 4;
            blockquantity = (blocksize - blockfirst) / 6;
            blockremainder = (blocksize - blockfirst) % 6;
            if (blockremainder != 0) {
                blockquantity++;
            }
        } else {
            blockfirst = blocksize;
            blockquantity = 0;
            blockremainder = 0;
        }

        data[0] = 0x40 + blockquantity; // e.g 0x40 send, | 0x0A (10) messages to follow
        data[1] = 0xA1;
        data[2] = blocksize + 1; // length+1 (64 Bytes)
        data[3] = 0x36; // Data Transfer
        if (!fread((data+4),1,blockfirst,fp)) {
            printf("\nError reading the BIN file MODIFIED.BIN\r\n");
            return FALSE;
        }
#ifdef DEBUG
// DEBUG info...
        for (k = 0; k < 8; k++ ) printf("0x%02X ", data[k] );
        for (k = 2; k < 8; k++ ) printf("%c ", data[k] );
        printf(" data\r\n");
#endif
        if (!can_send_timeout (T7SEC_ID, data, 8, T7MESSAGETIMEOUT)) {
            printf("\nerr %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        if (blocksize > 4) {
            for (i = (blockquantity-1); i>=0; i--) {
                data[0] = i;
                if ((i == 0) && (blockremainder != 0)) {
                    blockduring = blockremainder;
                } else {
                    blockduring = 6;
                }
                if (!fread((data+2),1,blockduring,fp)) {
                    printf("\nError reading the BIN file MODIFIED.BIN\r\n");
                    return FALSE;
                }
#ifdef DEBUG
//  DEBUG info...
                for (k = 0; k < 8; k++ ) printf("0x%02X ", data[k] );
                for (k = 2; k < 8; k++ ) printf("%c ", data[k] );
                printf(" data\r\n");
                printf("%6.2f\r", 100*(float)current_address/(float)T7FLASHSIZE );
#endif
                wait_us(300);
//                wait_ms(10);   // 31/3/12 this longer wait might be needed for i-bus connections...
                if (!can_send_timeout (T7SEC_ID, data, 8, T7MESSAGETIMEOUT)) {
                    printf("\nerr %s line: %d\r\n", __FILE__, __LINE__ );
                    return FALSE;
                }
            }
        }
        if (!can_wait_timeout(T7SEC_RX, data, 8, T7LONGERTIMEOUT)) {
            printf("\nerr %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        // Send acknowledgement
        T7_flash_ack[3] = data[0] & 0xBF;
        if (!can_send_timeout (T7ACK_ID, T7_flash_ack, 8, T7MESSAGETIMEOUT)) {
            printf("\nerr %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        if ( data[3] != 0x76 ) {
            printf ("\n");
//#ifdef DEBUG
//  DEBUG info...
            for (k = 0; k < 8; k++ ) printf("%#04x ", data[k] );
            for (k = 2; k < 8; k++ ) printf("%c ", data[k] );
            printf(" data\r\n");
//#endif
            printf("Cannot Program Address %#010x.\r\n", current_address);
            printf("Block Size %#04x, First %#04x, Quantity %#04x, Remainder %#04x\r\n", blocksize, blockfirst, blockquantity, blockremainder);
            printf("\nerr %s line: %d\r\n", __FILE__, __LINE__ );
            return FALSE;
        }
        current_address += blocksize;
//        if (!(current_address % 0x80))
            printf("%6.2f\r", 100*(float)current_address/(float)T7FLASHSIZE );
    }
    if (blockmode == true) wait_ms(T7MESSAGETIMEOUT);
    return TRUE;
}

bool t7_flash(FILE *fp, bool blockmode)
{
    timer.reset();
    timer.start();
    printf("T7 program and caldata\r\n");
    if (t7_flash_segment(fp, 0x000000, 0x070000, blockmode)) {              // T7 program and caldata
        printf("\nAdaptation data 1\r\n");
        if (t7_flash_segment(fp, 0x070000, 0x002000, blockmode)) {          // Adaptation data 1
            printf("\nAdaptation data 2\r\n");
            if (t7_flash_segment(fp, 0x07C000, 0x00280, blockmode)) {       // Adaptation data 2
                printf("\nT7Suite watermark\r\n");
                if (t7_flash_segment(fp, 0x07FD00, 0x000020, blockmode)) {      // T7Suite watermark
                    printf("\nT7 Header\r\n");
                    if (t7_flash_segment(fp, 0x07FF00, 0x000100, blockmode)) {  // T7 Header
                        timer.stop();
                        printf("100.00\r\n");
                        printf("SUCCESS! Programming the FLASH took %#.1f seconds.\r\n",timer.read());
                        return true;
                    }
                }
            }
        }
    }
    timer.stop();
    printf("\nFAILURE! Unable to program FLASH after %#.1f seconds.\r\n",timer.read());
    return false;
}

bool t7_recover(FILE *fp)
{
    timer.reset();
    timer.start();
    if (t7_flash_segment(fp, 0x000000, 0x080000, false)) {              // Entire T7 BIN File
        timer.stop();
        printf("100.00\r\n");
        printf("SUCCESS! Programming the FLASH took %#.1f seconds.\r\n",timer.read());
        return true;
    }
    timer.stop();
    printf("\nFAILURE! Unable to program FLASH after %#.1f seconds.\r\n",timer.read());
    return false;
}


/// Reset and restart a T7 ECU after a FLASH operation
///
/// NOTE: DOESN't WORK !!!
///
bool t7_reset()
{
    char T7_flash_exit[]    = T7FLAEXIT;
    char T7_flash_ack[]     = T7FLA_ACK;
    char T7_flash_end[]     = T7FLA_END;
    char data[8];

    printf("Restarting T7 ECU");
    // Send "Request Data Transfer Exit" to Trionic
    if (!can_send_timeout (T7SEC_ID, T7_flash_end, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    T7_flash_ack[3] = data[0] & 0xBF;
    if (!can_send_timeout (T7ACK_ID, T7_flash_ack, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if ( data[3] != 0x77 ) {
        printf("Cannot Update FLASH, message refused.\r\n");
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    //
    // Send "Request Data Transfer Exit" to Trionic
    if (!can_send_timeout (T7SEC_ID, T7_flash_exit, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if (!can_wait_timeout(T7SEC_RX, data, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    T7_flash_ack[3] = data[0] & 0xBF;
    if (!can_send_timeout (T7ACK_ID, T7_flash_ack, 8, T7MESSAGETIMEOUT)) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    if ( data[3] != 0x71 ) {
        printf("err %s line: %d\r\n", __FILE__, __LINE__ );
        return FALSE;
    }
    return true;
}

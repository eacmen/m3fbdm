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

#include "t8utils.h"

Timer   TesterPresent;


//
// t8_initialise
//
// sends an initialisation message to the T7 ECU
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//


bool t8_initialise()
{
    return TRUE;
}

bool t8_show_VIN()
{
    uint32_t i;
    char T8TxFlo[] = T8FLOCTL;
    char T8TxMsg[] = T8REQVIN;
    char T8RxMsg[8];
    printf("Requesting VIN from T8...\r\n");
    // Send "Request VIN" to Trionic8
    if (!can_send_timeout (T8TSTRID, T8TxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    // wait for the T8 to reply
    // Read "Seed"
    // if a message is not received id return false
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    //* DEBUG info...
    for (i = 0; i < 8; i++ ) printf("0x%02X ", T8RxMsg[i] );
    printf("\r\n");
    for (i = 5; i < 8; i++ ) printf("%c", T8RxMsg[i] );
    printf("\r\n");
    // Send Trionic8 a "Flow Control Message to get the rest of the VIN
    if (!can_send_timeout (T8TSTRID, T8TxFlo, 8, T8MESSAGETIMEOUT))
        return FALSE;
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    //* DEBUG info...
    for (i = 0; i < 8; i++ ) printf("0x%02X ", T8RxMsg[i] );
    printf("\r\n");
    for (i = 1; i < 8; i++ ) printf("%c", T8RxMsg[i] );
    printf("\r\n");
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    //* DEBUG info...
    for (i = 0; i < 8; i++ ) printf("0x%02X ", T8RxMsg[i] );
    printf("\r\n");
    for (i = 1; i < 8; i++ ) printf("%c", T8RxMsg[i] );
    printf("\r\n");
    //*/
    return TRUE;
}

bool t8_write_VIN()
{

    char SetVin10[] = {0x10,0x13,0x3B,0x90,0x59,0x53,0x33,0x46};
    char SetVin21[] = {0x21,0x46,0x34,0x35,0x53,0x38,0x33,0x31};
//  char SetVin22[] = {0x22,0x30,0x30,0x32,0x33,0x34,0x30,0xaa};  // Original
    char SetVin22[] = {0x22,0x30,0x30,0x34,0x33,0x32,0x31,0x00};
    char T8RxMsg[8];
    char k = 0;

//    GMLANTesterPresent(T8REQID, T8RESPID);
//    wait_ms(2000);
//
//    printf("Requesting Security Access\r\n");
//    if (!t8_authenticate(0x01)) {
//        printf("Unable to get Security Access\r\n");
//        return FALSE;
//    }
//    printf("Security Access Granted\r\n");
//
//    GMLANTesterPresent(T8REQID, T8RESPID);
//    wait_ms(2000);
//
//    GMLANTesterPresent(T8REQID, T8RESPID);
//    wait_ms(2000);
//
    if (!can_send_timeout (T8TSTRID, SetVin10, 8, T8MESSAGETIMEOUT)) {
        printf("Unable to write VIN\r\n");
        return FALSE;
    }
    for (k = 0; k < 8; k++ ) printf("0x%02X ", SetVin10[k] );
    printf("\r\n");
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    for (k = 0; k < 8; k++ ) printf("0x%02X ", T8RxMsg[k] );
    printf("\r\n");
//    wait_ms(100);
    if (!can_send_timeout (T8TSTRID, SetVin21, 8, T8MESSAGETIMEOUT)) {
        printf("Unable to write VIN\r\n");
        return FALSE;
    }
    for (k = 0; k < 8; k++ ) printf("0x%02X ", SetVin21[k] );
    printf("\r\n");
//    wait_ms(100);
    if (!can_send_timeout (T8TSTRID, SetVin22, 8, T8MESSAGETIMEOUT)) {
        printf("Unable to write VIN\r\n");
        return FALSE;
    }
    for (k = 0; k < 8; k++ ) printf("0x%02X ", SetVin22[k] );
    printf("\r\n");
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    for (k = 0; k < 8; k++ ) printf("0x%02X ", T8RxMsg[k] );
    printf("\r\n");
    return TRUE;
//    GMLANTesterPresent(T8REQID, T8RESPID);
//    wait_ms(2000);
//
}

//
// t8_authenticate
//
// sends an authentication message to the T7 ECU
// but doesn't display anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//

bool t8_authenticate(uint32_t ReqID, uint32_t RespID, char level)
{
    uint16_t i, seed, key;
//    if (!GMLANSecurityAccessRequest(ReqID, RespID, level, seed)) {
//        printf("Unable to request SEED value for security access\r\n");
//        return FALSE;
//    }

    for (i=0; i < 20; i++) {
        if (GMLANSecurityAccessRequest(ReqID, RespID, level, seed))
            break;
        wait(1);
        GMLANTesterPresent(ReqID, RespID);
    }
    if (i == 20) {
        printf("Unable to request SEED value for security access\r\n");
        return FALSE;
    }

    if ( seed == 0x0000 ) {
        printf("T8 ECU is already unlocked\r\n");
        return TRUE;
    }
    key = (seed >> 5) | (seed << 11);
    key += 0xB988;
    if (level == 0xFD) {
        key /= 3;
        key ^= 0x8749;
        key += 0x0ACF;
        key ^= 0x81BF;
    } else if (level == 0xFB) {
        key ^= 0x8749;
        key += 0x06D3;
        key ^= 0xCFDF;
    }
    /* CIM KEY CALCULATION
        uint16_t key = (seed + 0x9130);
        key = (key >> 8) | (key << 8);
        key -= 0x3FC7;
    */
    wait_ms(1);
    if (!GMLANSecurityAccessSendKey(ReqID, RespID, level, key)) {
        printf("Unable to send KEY value for security access\r\n");
        return FALSE;
    }
    printf("Key Accepted\r\n");
    wait_ms(500);       // was 5
    return TRUE;
}


//
// t8_dump
//
// dumps the T8 BIN File
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//

bool t8_dump()
{
    uint32_t i = 0, k = 0;
    char T8TxMsg[8];
    char T8RxMsg[8];

    timer.reset();
    timer.start();
    printf("Creating FLASH dump file...\r\n");

//
    if (!GMLANprogrammingSetupProcess(T8REQID, T8RESPID))
        return FALSE;
//
    printf("Requesting Security Access\r\n");
    if (!t8_authenticate(T8REQID, T8RESPID, 0x01)) {
        printf("Unable to get Security Access\r\n");
        return FALSE;
    }
    printf("Security Access Granted\r\n");
//
    if(!GMLANprogrammingUtilityFileProcess(T8REQID, T8RESPID, T8BootloaderRead))
        return FALSE;
//
//
    printf("Downloading FLASH BIN file...\r\n");
    printf("Creating FLASH dump file...\r\n");
    FILE *fp = fopen("/local/original.bin", "w");    // Open "original.bin" on the local file system for writing
    if (!fp) {
        perror ("The following error occured");
        return TERM_ERR;
    }
    printf("  0.00 %% complete.\r");
    TesterPresent.start();

// It is possible to save some time by only reading the program code and CAL data
// This is just a rough calculation, and slight overestimate of the number of blocks of data needed to send the BIN file
    T8TxMsg[0] = 0x06;
    T8TxMsg[1] = 0x21;
    T8TxMsg[2] = 0x80;  // Blocksize
    T8TxMsg[3] = 0x00;  // This address (0x020140) points to the Header at the end of the BIN
    T8TxMsg[4] = 0x02;
    T8TxMsg[5] = 0x01;
    T8TxMsg[6] = 0x40;
    T8TxMsg[7] = 0xaa;
    if (!can_send_timeout (T8TSTRID, T8TxMsg, 7, T8MESSAGETIMEOUT)) {
        printf("Unable to download FLASH\r\n");
        return FALSE;
    }
    if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
        return FALSE;
    uint32_t EndAddress = (T8RxMsg[5] << 16) | (T8RxMsg[6] << 8) | T8RxMsg[7];
    EndAddress += 0x200;    // Add some bytes for the Footer itself and to account for division rounded down later
    char T8TxFlo[] = T8FLOCTL;
    can_send_timeout (T8TSTRID, T8TxFlo, 8, T8MESSAGETIMEOUT);
    for (i = 0; i < 0x12; i++) {
        if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
            return FALSE;
    }
    printf("Reading your BIN file adjusted for footer = 0x%06X Bytes\r\n", EndAddress );

    for ( uint32_t StartAddress = 0x0; StartAddress < EndAddress; StartAddress +=0x80 ) {     // 0x100000
        T8TxMsg[0] = 0x06;
        T8TxMsg[1] = 0x21;
        T8TxMsg[2] = 0x80;  // Blocksize
        T8TxMsg[3] = (char) (StartAddress >> 24);
        T8TxMsg[4] = (char) (StartAddress >> 16);
        T8TxMsg[5] = (char) (StartAddress >> 8);
        T8TxMsg[6] = (char) (StartAddress);
        T8TxMsg[7] = 0xaa;
#ifdef DEBUG
        printf("block %#.3f\r\n",timer.read());
#endif
        if (!can_send_timeout (T8TSTRID, T8TxMsg, 7, T8MESSAGETIMEOUT)) {
            printf("Unable to download FLASH\r\n");
            return FALSE;
        }
        if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
            return FALSE;
#ifdef DEBUG
        printf("first %#.3f\r\n",timer.read());
#endif
        uint32_t txpnt = 0;
        for (k = 4; k < 8; k++ ) file_buffer[txpnt++] = T8RxMsg[k];

        uint8_t DataFrames = 0x12;
        char iFrameNumber = 0x21;
        char T8TxFlo[] = T8FLOCTL;
        can_send_timeout (T8TSTRID, T8TxFlo, 8, T8MESSAGETIMEOUT);
#ifdef DEBUG
        printf("flowCtrl %#.3f\r\n",timer.read());
#endif
        for (i = 0; i < DataFrames; i++) {
            if (!can_wait_timeout(T8ECU_ID, T8RxMsg, 8, T8MESSAGETIMEOUT))
                return FALSE;
#ifdef DEBUG
            printf("Consec %#.3f\r\n",timer.read());
#endif
            iFrameNumber++;
            for (k = 1; k < 8; k++ ) file_buffer[txpnt++] = T8RxMsg[k];
        }
        fwrite((file_buffer), 1, 0x80, fp);
        if (ferror (fp)) {
            fclose (fp);
            printf ("Error writing to the FLASH BIN file.\r\n");
            return TERM_ERR;
        }
        printf("%6.2f\r", (100.0*(float)StartAddress)/(float)(EndAddress) );
        if (TesterPresent.read_ms() > 2000) {
            GMLANTesterPresent(T8REQID, T8RESPID);
            TesterPresent.reset();
        }
    }

    for (uint32_t i = 0; i < 0x80; i++)
        file_buffer[i] = 0xFF;
    while ( ftell(fp) < 0x100000 ) {
//  for ( uint32_t StartAddress = EndAddress; StartAddress < 0x100000; StartAddress +=0x80 ) {
        fwrite((file_buffer), 1, 0x80, fp);
        if (ferror (fp)) {
            fclose (fp);
            printf ("Error writing to the FLASH BIN file.\r\n");
            return TERM_ERR;
        }
    }

    printf("%6.2f\r\n", (float)100 );
    timer.stop();
    printf("SUCCESS! Getting the FLASH dump took %#.1f seconds.\r\n",timer.read());
    fclose(fp);
    return TRUE;
}


bool t8_flash()
{
    uint32_t i = 0, j = 0, k = 0;

    timer.reset();
    timer.start();
    printf("FLASHing T8 BIN file...\r\n");

//
    if (!GMLANprogrammingSetupProcess(T8REQID, T8RESPID))
        return FALSE;
//
    printf("Requesting Security Access\r\n");
    if (!t8_authenticate(T8REQID, T8RESPID, 0x01)) {
        printf("Unable to get Security Access\r\n");
        return FALSE;
    }
    printf("Security Access Granted\r\n");
//
// All steps needed to transfer and start a bootloader ('Utility File' in GMLAN parlance)
//    const uint8_t BootLoader[] = T8BootloaderProg;
//    if(!GMLANprogrammingUtilityFileProcess(T8REQID, T8RESPID, BootLoader))
    if(!GMLANprogrammingUtilityFileProcess(T8REQID, T8RESPID, T8BootLoaderWrite))
        return FALSE;
//
    uint32_t StartAddress = 0x020000;
    uint32_t txpnt = 0;
    char iFrameNumber = 0x21;
    char GMLANMsg[8];
    char data2Send[0xE0];
//
    // fopen modified.bin here, check it is OK and work out how much data I need to send
    // need lots of fcloses though
    printf("Checking the FLASH BIN file...\r\n");
    FILE *fp = fopen("/local/modified.bin", "r");    // Open "modified.bin" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the BIN file MODIFIED.BIN\r\n");;
        return TERM_ERR;
    }
    // obtain file size - it should match the size of the FLASH chips:
    fseek (fp , 0 , SEEK_END);
    uint32_t file_size = ftell (fp);
    rewind (fp);

    // read the initial stack pointer value in the BIN file - it should match the value expected for the type of ECU
    uint32_t stack_long = 0;
    if (!fread(&stack_long,4,1,fp)) {
        fclose(fp);
        return TERM_ERR;
    }
    stack_long = (stack_long >> 24) | ((stack_long << 8) & 0x00FF0000) | ((stack_long >> 8) & 0x0000FF00) |  (stack_long << 24);
//
    if (file_size != T8FLASHSIZE || stack_long != T8POINTER) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T8 ECU :-(\r\n");
        printf("BIN file size: %#010x, FLASH chip size: %#010x, Pointer: %#010x.\r\n", file_size, T7FLASHSIZE, stack_long);
        return TERM_ERR;
    }
// It is possible to save some time by only sending the program code and CAL data
// This is just a rough calculation, and slight overestimate of the number of blocks of data needed to send the BIN file
    uint32_t blocks2Send;
    fseek(fp,0x020140,SEEK_SET);
    if (!fread(&blocks2Send,4,1,fp)) {
        fclose(fp);
        return TERM_ERR;
    }
    blocks2Send = (blocks2Send >> 24) | ((blocks2Send << 8) & 0x00FF0000) | ((blocks2Send >> 8) & 0x0000FF00) |  (blocks2Send << 24);
    printf("Start address of BIN file's Footer area = 0x%06X\r\n", blocks2Send );
    blocks2Send += 0x200;       // Add some bytes for the Footer itself and to account for division rounded down later
    blocks2Send -= 0x020000;    // Remove 0x020000 because we don't send the bootblock and adaptation blocks
    printf("Amount of data to send BIN file adjusted for footer = 0x%06X Bytes\r\n", blocks2Send );
    blocks2Send /= 0xE0;
    printf("Number of Blocks of 0xE0 Bytes needed to send BIN file = 0x%04X\r\n", blocks2Send );
// Move BIN file pointer to start of data
    fseek (fp , 0x020000 , SEEK_SET);
// Erase the FLASH
    printf("Waiting for FLASH to be Erased\r\n");
    if (!GMLANRequestDownload(T8REQID, T8RESPID, GMLANRequestDownloadModeEncrypted)) {
        fclose(fp);
        printf("Unable to erase the FLASH chip!\r\n");
        return FALSE;
    }
// Now send the BIN file
    GMLANTesterPresent(T8REQID, T8RESPID);
    TesterPresent.start();
    printf("Sending FLASH BIN file\r\n");
    printf("  0.00 %% complete.\r");
    for (i=0; i<blocks2Send; i++) {
        // get a block of 0xE0 bytes in an array called data2Send
        if (!fread(data2Send,0xE0,1,fp)) {
            fclose(fp);
            printf("\r\nError reading the BIN file MODIFIED.BIN\r\n");
            return FALSE;
        }
        // encrypt data2Send array by XORing with 6 different values in a ring (modulo function)
        char key[6] = { 0x39, 0x68, 0x77, 0x6D, 0x47, 0x39 };
        for ( j = 0; j < 0xE0; j++ )
            data2Send[j] ^= key[(((0xE0*i)+j) % 6)];
        // Send the block of data
        if (!GMLANDataTransferFirstFrame(T8REQID, T8RESPID, 0xE6, GMLANDOWNLOAD, StartAddress)) {
            fclose(fp);
            printf("\r\nUnable to start BIN File Upload\r\n");
            return FALSE;
        }
        // Send 0x20 messages of 0x07 bytes each (0x20 * 0x07 = 0xE0)
        txpnt = 0;
        iFrameNumber = 0x21;
        for (j=0; j < 0x20; j++) {
            GMLANMsg[0] = iFrameNumber;
            for (k=1; k<8; k++)
                GMLANMsg[k] = data2Send[txpnt++];
            if (!can_send_timeout(T8REQID, GMLANMsg, 8, GMLANPTCT)) {
                fclose(fp);
                printf("\r\nUnable to send BIN File\r\n");
                return FALSE;
            }
            ++iFrameNumber &= 0x2F;
            wait_us(1000);   // can be as low as 250 for an ECU on its own, but need 1000 (1ms) to work in a car (use a longer delay to be ultrasafe??? )
        }
        if (!GMLANDataTransferBlockAcknowledge(T8RESPID)) {
            fclose(fp);
            return FALSE;
        }
        if (TesterPresent.read_ms() > 2000) {
            GMLANTesterPresent(T8REQID, T8RESPID);
            TesterPresent.reset();
        }
        StartAddress += 0xE0;
        printf("%6.2f\r", (100.0*(float)i)/(float)(blocks2Send) );
    }
// FLASHing complete
    printf("%6.2f\r\n", (float)100 );
// End programming session and return to normal mode
    if (!GMLANReturnToNormalMode(T8REQID, T8RESPID)) {
        fclose(fp);
        printf("UH-OH! T8 ECU did not Return To Normal Mode!!\r\n");
        return FALSE;
    }
    timer.stop();
    printf("SUCCESS! FLASHing the BIN file took %#.1f seconds.\r\n",timer.read());
    fclose(fp);
    return TRUE;
}

bool t8_recover()
{
    uint32_t i = 0, j = 0, k = 0;

    timer.reset();
    timer.start();
    printf("Recovering your T8 ECU ...\r\n");
//
    if (!GMLANprogrammingSetupProcess(T8USDTREQID, T8UUDTRESPID))
        return FALSE;
//
    printf("Requesting Security Access\r\n");
    if (!t8_authenticate(T8USDTREQID, T8UUDTRESPID, 0x01)) {
        printf("Unable to get Security Access\r\n");
        return FALSE;
    }
    printf("Security Access Granted\r\n");
//
//    const uint8_t BootLoader[] = T8BootloaderProg;
//    if(!GMLANprogrammingUtilityFileProcess(T8USDTREQID, T8UUDTRESPID, BootLoader))
    if(!GMLANprogrammingUtilityFileProcess(T8USDTREQID, T8UUDTRESPID, T8BootLoaderWrite))
        return FALSE;
//


// All steps needed to transfer and start a bootloader ('Utility File' in GMLAN parlance)
    uint32_t StartAddress = 0x020000;
    uint32_t txpnt = 0;
    char iFrameNumber = 0x21;
    char GMLANMsg[8];
    char data2Send[0xE0];
//
    // fopen modified.bin here, check it is OK and work out how much data I need to send
    // need lots of fcloses though
    printf("Checking the FLASH BIN file...\r\n");
    FILE *fp = fopen("/local/modified.bin", "r");    // Open "modified.bin" on the local file system for reading
    if (!fp) {
        printf("Error: I could not find the BIN file MODIFIED.BIN\r\n");;
        return TERM_ERR;
    }
    // obtain file size - it should match the size of the FLASH chips:
    fseek (fp , 0 , SEEK_END);
    uint32_t file_size = ftell (fp);
    rewind (fp);

    // read the initial stack pointer value in the BIN file - it should match the value expected for the type of ECU
    uint32_t stack_long = 0;
    if (!fread(&stack_long,4,1,fp)) {
        fclose(fp);
        return TERM_ERR;
    }
    stack_long = (stack_long >> 24) | ((stack_long << 8) & 0x00FF0000) | ((stack_long >> 8) & 0x0000FF00) |  (stack_long << 24);
//
    if (file_size != T8FLASHSIZE || stack_long != T8POINTER) {
        fclose(fp);
        printf("The BIN file does not appear to be for a T8 ECU :-(\r\n");
        printf("BIN file size: %#010x, FLASH chip size: %#010x, Pointer: %#010x.\r\n", file_size, T7FLASHSIZE, stack_long);
        return TERM_ERR;
    }
// It is possible to save some time by only sending the program code and CAL data
// This is just a rough calculation, and slight overestimate of the number of blocks of data needed to send the BIN file
    uint32_t blocks2Send;
    fseek(fp,0x020140,SEEK_SET);
    if (!fread(&blocks2Send,4,1,fp)) {
        fclose(fp);
        return TERM_ERR;
    }
    blocks2Send = (blocks2Send >> 24) | ((blocks2Send << 8) & 0x00FF0000) | ((blocks2Send >> 8) & 0x0000FF00) |  (blocks2Send << 24);
    printf("Start address of BIN file's Footer area = 0x%06X\r\n", blocks2Send );
    blocks2Send += 0x200;       // Add some bytes for the Footer itself and to account for division rounded down later
    blocks2Send -= 0x020000;    // Remove 0x020000 because we don't send the bootblock and adaptation blocks
    printf("Amount of data to send BIN file adjusted for footer = 0x%06X Bytes\r\n", blocks2Send );
    blocks2Send /= 0xE0;
    printf("Number of Blocks of 0xE0 Bytes needed to send BIN file = 0x%04X\r\n", blocks2Send );
// Move BIN file pointer to start of data
    fseek (fp , 0x020000 , SEEK_SET);
// Erase the FLASH
    printf("Waiting for FLASH to be Erased\r\n");
    if (!GMLANRequestDownload(T8REQID, T8RESPID, GMLANRequestDownloadModeEncrypted)) {
        fclose(fp);
        printf("Unable to erase the FLASH chip!\r\n");
        return FALSE;
    }
// Now send the BIN file
    GMLANTesterPresent(T8REQID, T8RESPID);
    TesterPresent.start();
    printf("Sending FLASH BIN file\r\n");
    printf("  0.00 %% complete.\r");
    for (i=0; i<blocks2Send; i++) {
        // get a block of 0xE0 bytes in an array called data2Send
        if (!fread(data2Send,0xE0,1,fp)) {
            fclose(fp);
            printf("\r\nError reading the BIN file MODIFIED.BIN\r\n");
            return FALSE;
        }
        // encrypt data2Send array by XORing with 6 different values in a ring (modulo function)
        char key[6] = { 0x39, 0x68, 0x77, 0x6D, 0x47, 0x39 };
        for ( j = 0; j < 0xE0; j++ )
            data2Send[j] ^= key[(((0xE0*i)+j) % 6)];
        // Send the block of data
        if (!GMLANDataTransferFirstFrame(T8REQID, T8RESPID, 0xE6, GMLANDOWNLOAD, StartAddress)) {
            fclose(fp);
            printf("\r\nUnable to start BIN File Upload\r\n");
            return FALSE;
        }
        // Send 0x20 messages of 0x07 bytes each (0x20 * 0x07 = 0xE0)
        txpnt = 0;
        iFrameNumber = 0x21;
        for (j=0; j < 0x20; j++) {
            GMLANMsg[0] = iFrameNumber;
            for (k=1; k<8; k++)
                GMLANMsg[k] = data2Send[txpnt++];
            if (!can_send_timeout(T8REQID, GMLANMsg, 8, GMLANPTCT)) {
                fclose(fp);
                printf("\r\nUnable to send BIN File\r\n");
                return FALSE;
            }
            ++iFrameNumber &= 0x2F;
            wait_us(1000);   // was 250 use 1000 or wait_ms(1) to be ultrasafe
        }
        if (!GMLANDataTransferBlockAcknowledge(T8RESPID)) {
            fclose(fp);
            return FALSE;
        }
        if (TesterPresent.read_ms() > 2000) {
            GMLANTesterPresent(T8REQID, T8RESPID);
            TesterPresent.reset();
        }
        StartAddress += 0xE0;
        printf("%6.2f\r", (100.0*(float)i)/(float)(blocks2Send) );
    }
// FLASHing complete
    printf("%6.2f\r\n", (float)100 );
// End programming session and return to normal mode
    if (!GMLANReturnToNormalMode(T8REQID, T8RESPID)) {
        fclose(fp);
        printf("UH-OH! T8 ECU did not Return To Normal Mode!!\r\n");
        return FALSE;
    }
    timer.stop();
    fclose(fp);
    printf("SUCCESS: Your T8 ECU has been recovered.\r\n");
    return TRUE;
}

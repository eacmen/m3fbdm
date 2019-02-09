/*******************************************************************************

canutils.cpp
(c) 2010, 2012 by Sophie Dexter

General purpose CAN bus functions for Just4Trionic by Just4pLeisure
Functions that work with the CAN bus directly. Anything to do with the CAN bus
must (should anyway) be done by one of these functions.

********************************************************************************

WARNING: Use at your own risk, sadly this software comes with no guarantees.
This software is provided 'free' and in good faith, but the author does not
accept liability for any damage arising from its use.

*******************************************************************************/

#include "canutils.h"

//CAN can2(p30, p29);
// Use a timer to see if things take too long
Timer CANTimer;


//LPC_CANx->MOD |= 1;          // Disble CAN controller 2
//LPC_CANx->MOD |= (1 << 1);   // Put into listen only mode
//LPC_CANx->MOD &= ~(1);       // Re-enable CAN controller

void can_disable(uint8_t chan)
{
    // Put a CAN controller into disabled condition
    chan == 1 ? LPC_CAN1->MOD |= 1 : LPC_CAN2->MOD |= 1;
}

void can_enable(uint8_t chan)
{
    // Put a CAN controller in operating mode
    chan == 1 ? LPC_CAN1->MOD &= ~(1) : LPC_CAN2->MOD &= ~(1);
}

void can_configure(uint8_t chan, uint32_t baud, bool listen)
{

    LPC_CAN_TypeDef *pCANx = (chan == 1) ? LPC_CAN1 : LPC_CAN2;

    uint32_t result;
    uint8_t TQU, TSEG1=0, TSEG2=0;
    uint16_t BRP=0;

    switch (chan) {
        case 1:
            LPC_SC->PCONP       |=  (1 << 13);          // Enable CAN controller 1
            LPC_PINCON->PINSEL0 |=  (1 <<  0);          // Pin 9 (port0 bit0) used as Receive
            LPC_PINCON->PINSEL0 |=  (1 <<  2);          // Pin 10 (port0 bit1) used as Transmit
            break;
        case 2:
        default:
            LPC_SC->PCONP       |=  (1 << 14);          // Enable CAN controller 2
            LPC_PINCON->PINSEL0 |=  (1 <<  9);          // Pin 30 (port0 bit4) used as Receive
            LPC_PINCON->PINSEL0 |=  (1 << 11);          // Pin 29 (port0 bit5) used as Transmit
            break;
    }
    pCANx->MOD = 0x01;                                  // Put into reset mode

    pCANx->IER   = 0;                                   // Disable all interrupts
    pCANx->GSR   = 0;                                   // Clear status register
    pCANx->CMR = (1<<1)|(1<<2)|(1<<3);                  // Clear Receive path, abort anything waiting to send and clear errors
    // CANx->CMR = CAN_CMR_AT | CAN_CMR_RRB | CAN_CMR_CDO;
    result = pCANx->ICR;                                // Read interrupt register to clear it

    // Calculate a suitable BTR register setting
    // The Bit Rate Pre-scaler (BRP) can be in the range of 1-1024
    // Bit Time can be be between 25 and 8 Time Quanta (TQU) according to CANopen
    // Bit Time = SyncSeg(1) + TSEG1(1-16) + TSEG2(1-8)
    // SyncSeg is always 1TQU, TSEG2 must be at least 2TQU to be longer than the processing time
    // Opinions vary on when to take a sample but a point roughly 2/3 of Bit Time seems OK, TSEG1 is roughly 2*TSEG2
    // Synchronisation Jump width can be 1-4 TQU, a value of 1 seems to be normal
    // All register values are -1, e.g. TSEG1 can range from 1-16 TQU, so register values are 0-15 to fit into 4 bits
    result =  CANsuppliedCLK / baud;
    for (TQU=25; TQU>7; TQU--) {
        if ((result%TQU)==0) {
            BRP = (result/TQU) - 1;
            TSEG2 = (TQU/3) - 1;                        // Subtract 1
            TSEG1 = TQU - (TQU/3) - 2;                  // Subtract 2 to allow for SyncSeg
            break;
        }
    }
    pCANx->BTR  = (TSEG2<<20)|(TSEG1<<16)|(0<<14)|BRP;  // Set bit timing, SAM = 0, TSEG2, TSEG1, SJW = 1 (0+1), BRP

    can_reset_filters();                                // Initialise the Acceptance Filters
    can_use_filters(FALSE);                             // Accept all messages (Acceptance Filters disabled)
    // Go :-)
    can_rs_pin = listen;                                // enable/disable CAN driver chip
    pCANx->MOD = (listen <<1);                          // Enable CAN controller in active/listen mode
}


void can_reset_filters()
{
    // Initialise the Acceptance Filters
    LPC_CANAF->AFMR = 0x01;                             // Put Acceptance Filter into reset/configuration mode
    for (uint16_t i = 0; i < 512; i++)
        LPC_CANAF_RAM->mask[i] = 0x00;                  // Clear the Acceptance Filter memory
    LPC_CANAF->SFF_sa = 0x00;                           // Clear the Acceptance Filter registers
    LPC_CANAF->SFF_GRP_sa = 0x00;
    LPC_CANAF->EFF_sa = 0x00;
    LPC_CANAF->EFF_GRP_sa = 0x00;
    LPC_CANAF->ENDofTable = 0x00;
    LPC_CANAF->AFMR = 0x00;                             // Enable Acceptance Filter all messages should be rejected
}

void can_use_filters(bool active)
{
    active ? LPC_CANAF->AFMR = 0 : LPC_CANAF->AFMR = 2;
}

/*--------------------------------------------
  setup acceptance filter for CAN controller 2
  original http://www.dragonwake.com/download/LPC1768/Example/CAN/CAN.c
  simplified for CAN2 interface and std id (11 bit) only
 *--------------------------------------------*/
void can_add_filter(uint8_t chan, uint32_t id)
{

    static int CAN_std_cnt = 0;
    uint32_t buf0, buf1;
    int cnt1, cnt2, bound1;

    /* Acceptance Filter Memory full */
    if (((CAN_std_cnt + 1) >> 1) >= 512)
        return;                                         // error: objects full

    /* Setup Acceptance Filter Configuration
      Acceptance Filter Mode Register = Off  */
    LPC_CANAF->AFMR = 0x00000001;

    id &= 0x000007FF;                                   // Mask out 16-bits of ID
    id |= (chan-1) << 13;                               // Add CAN controller number (1 or 2)

    if (CAN_std_cnt == 0)  {                     /* For entering first  ID */
        LPC_CANAF_RAM->mask[0] = 0x0000FFFF | (id << 16);
    }  else if (CAN_std_cnt == 1)  {             /* For entering second ID */
        if ((LPC_CANAF_RAM->mask[0] >> 16) > id)
            LPC_CANAF_RAM->mask[0] = (LPC_CANAF_RAM->mask[0] >> 16) | (id << 16);
        else
            LPC_CANAF_RAM->mask[0] = (LPC_CANAF_RAM->mask[0] & 0xFFFF0000) | id;
    }  else  {
        /* Find where to insert new ID */
        cnt1 = 0;
        cnt2 = CAN_std_cnt;
        bound1 = (CAN_std_cnt - 1) >> 1;
        while (cnt1 <= bound1)  {                  /* Loop through standard existing IDs */
            if ((LPC_CANAF_RAM->mask[cnt1] >> 16) > id)  {
                cnt2 = cnt1 * 2;
                break;
            }
            if ((LPC_CANAF_RAM->mask[cnt1] & 0x0000FFFF) > id)  {
                cnt2 = cnt1 * 2 + 1;
                break;
            }
            cnt1++;                                  /* cnt1 = U32 where to insert new ID */
        }                                          /* cnt2 = U16 where to insert new ID */

        if (cnt1 > bound1)  {                      /* Adding ID as last entry */
            if ((CAN_std_cnt & 0x0001) == 0)         /* Even number of IDs exists */
                LPC_CANAF_RAM->mask[cnt1]  = 0x0000FFFF | (id << 16);
            else                                     /* Odd  number of IDs exists */
                LPC_CANAF_RAM->mask[cnt1]  = (LPC_CANAF_RAM->mask[cnt1] & 0xFFFF0000) | id;
        }  else  {
            buf0 = LPC_CANAF_RAM->mask[cnt1];        /* Remember current entry */
            if ((cnt2 & 0x0001) == 0)                /* Insert new mask to even address */
                buf1 = (id << 16) | (buf0 >> 16);
            else                                     /* Insert new mask to odd  address */
                buf1 = (buf0 & 0xFFFF0000) | id;

            LPC_CANAF_RAM->mask[cnt1] = buf1;        /* Insert mask */

            bound1 = CAN_std_cnt >> 1;
            /* Move all remaining standard mask entries one place up */
            while (cnt1 < bound1)  {
                cnt1++;
                buf1  = LPC_CANAF_RAM->mask[cnt1];
                LPC_CANAF_RAM->mask[cnt1] = (buf1 >> 16) | (buf0 << 16);
                buf0  = buf1;
            }

            if ((CAN_std_cnt & 0x0001) == 0)         /* Even number of IDs exists */
                LPC_CANAF_RAM->mask[cnt1] = (LPC_CANAF_RAM->mask[cnt1] & 0xFFFF0000) | (0x0000FFFF);
        }
    }
    CAN_std_cnt++;

    /* Calculate std ID start address (buf0) and ext ID start address <- none (buf1) */
    buf0 = ((CAN_std_cnt + 1) >> 1) << 2;
    /* Setup acceptance filter pointers */
    LPC_CANAF->SFF_sa     = 0;
    LPC_CANAF->SFF_GRP_sa = buf0;
    LPC_CANAF->EFF_sa     = buf0;
    LPC_CANAF->EFF_GRP_sa = buf0;
    LPC_CANAF->ENDofTable = buf0;

    LPC_CANAF->AFMR = 0x00000000;                  /* Use acceptance filter */
} // CAN2_wrFilter


void can_open()
{
    // activate external can transceiver
    can.reset();
    can_rs_pin = 0;
}

void can_close()
{
    // disable external can transceiver
    can_rs_pin = 1;
    can.reset();
}

void can_monitor()
{
    // Put CAN into silent monitoring mode
    can.monitor(1);
}

void can_active()
{
    // Take CAN out of silent monitoring mode
    can.monitor(0);
}

uint8_t can_set_speed(uint32_t speed)
{
    // 600kbit/s first - basically sets up CAN interface, but to wrong speed - not sure what else it does
//    can.frequency(600000);
    // 615kbit/s direct write of 615 kbit/s speed setting
//    LPC_CAN2->BTR = 0x370002;
    return (can.frequency(speed)) ? TERM_OK : TERM_ERR;
}

//
// show_can_message
//
// Displays a CAN message in the RX buffer if there is one.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
extern void show_can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        CANRXLEDON;
        printf("w%03x%d", can_MsgRx.id, can_MsgRx.len);
        for (char i=0; i<can_MsgRx.len; i++)
            printf("%02x", can_MsgRx.data[i]);
        //printf(" %c ", can_MsgRx.data[2]);
        printf("\r\n");
    }
    return;
}

//
// show_T5can_message
//
// Displays a Trionic 5 CAN message in the RX buffer if there is one.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
extern void show_T5can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        CANRXLEDON;
        switch (can_MsgRx.id) {
            case 0x005:
            case 0x006:
            case 0x00C:
            case 0x008:
                printf("w%03x%d", can_MsgRx.id, can_MsgRx.len);
                for (char i=0; i<can_MsgRx.len; i++)
                    printf("%02x", can_MsgRx.data[i]);
                printf("\r\n");
                break;
        }
    }
    return;
}
//
// show_T7can_message
//
// Displays a Trionic 7 CAN message in the RX buffer if there is one.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
extern void show_T7can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        CANRXLEDON;
        switch (can_MsgRx.id) {
            case 0x1A0:         //1A0h - Engine information
            case 0x280:         //280h - Pedals, reverse gear
            case 0x290:         //290h - Steering wheel and SID buttons
            case 0x2F0:         //2F0h - Vehicle speed
            case 0x320:         //320h - Doors, central locking and seat belts
            case 0x370:         //370h - Mileage
            case 0x3A0:         //3A0h - Vehicle speed
            case 0x3B0:         //3B0h - Head lights
            case 0x3E0:         //3E0h - Automatic Gearbox
            case 0x410:         //410h - Light dimmer and light sensor
            case 0x430:         //430h - SID beep request (interesting for Knock indicator?)
            case 0x460:         //460h - Engine rpm and speed
            case 0x4A0:         //4A0h - Steering wheel, Vehicle Identification Number
            case 0x520:         //520h - ACC, inside temperature
            case 0x530:         //530h - ACC
            case 0x5C0:         //5C0h - Coolant temperature, air pressure
            case 0x630:         //630h - Fuel usage
            case 0x640:         //640h - Mileage
            case 0x7A0:         //7A0h - Outside temperature
                printf("w%03x%d", can_MsgRx.id, can_MsgRx.len);
                for (char i=0; i<can_MsgRx.len; i++)
                    printf("%02x", can_MsgRx.data[i]);
                //printf(" %c ", can_MsgRx.data[2]);
                printf("\r\n");
                break;
        }
    }
    return;
}
//
// show_T8can_message
//
// Displays a Trionic 8 CAN message in the RX buffer if there is one.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
extern void show_T8can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        CANRXLEDON;
        switch (can_MsgRx.id) {
            case 0x645: // CIM
            case 0x7E0:
            case 0x7E8:
            case 0x311:
            case 0x5E8:
                //case 0x101:
                printf("w%03x%d", can_MsgRx.id, can_MsgRx.len);
                for (char i=0; i<can_MsgRx.len; i++)
                    printf("%02x", can_MsgRx.data[i]);
                printf("\r\n");
                break;
        }
    }
    return;
}

//
// silent_can_message
//
// Turns on the CAN receive LED if there is a CAN message
// but doesn't displays anything.
//
// inputs:    none
// return:    bool TRUE if there was a message, FALSE if no message.
//
extern void silent_can_message()
{
    CANMessage can_MsgRx;
    if (can.read(can_MsgRx)) {
        CANRXLEDON;
    }
    return;
}

//
// Sends a CAN Message, returns FALSE if the message wasn't sent in time
//
// inputs:  integer CAN message 'id', pointer to 'frame', integer message length and integer timeout
// return:     TRUE if the CAN message was sent before the 'timeout' expires
//          FALSE if 'timeout' expires or the message length is wrong
//
extern bool can_send_timeout (uint32_t id, char *frame, uint8_t len, uint16_t timeout)
{
    CANTimer.reset();
    CANTimer.start();
#ifdef DEBUG
    printf("ID:%03x Len:%03x", id, len);
    for (char i=0; i<len; i++) {
        printf(" %02x", frame[i]);
    }
    printf("\n\r");
#endif
    while (CANTimer.read_ms() < timeout) {
        if (can.write(CANMessage(id, frame, len))) {
            CANTimer.stop();
            CANTXLEDON;
//            led1 = 1;
            return TRUE;
        }
    }
    can.reset();
    CANTimer.stop();
    return FALSE;
}

//
// Waits for a CAN Message with the specified 'id' for a time specified by the 'timeout'
// All other messages are ignored
// The CAN message frame is returned using the pointer to 'frame'
//
// inputs:    integer CAN message 'id', pointer to 'frame' for returning the data
//          integer expected length of message, len and integer for the waiting time 'timeout'
//
// return:    TRUE if a qualifying message was received
//          FALSE if 'timeout' expires or the message length is wrong
//
extern bool can_wait_timeout (uint32_t id, char *frame, uint8_t len, uint16_t timeout)
{
    CANMessage CANMsgRx;
    CANTimer.reset();
    CANTimer.start();
    while (CANTimer.read_ms() < timeout) {
        if (can.read(CANMsgRx)) {
#ifdef DEBUG
            printf("ID:%03x Len:%03x", CANMsgRx.id, CANMsgRx.len);
            for (char i=0; i<len; i++) {
                printf(" %02x", CANMsgRx.data[i]);
            }
            printf("\n\r");
#endif
            CANRXLEDON;
//            led2 = 1;
            if (CANMsgRx.id == id || id ==0) {
                CANTimer.stop();
//                if (CANMsgRx.len != len)
//                    return FALSE;
                for (int i=0; i<len; i++)
                    frame[i] = CANMsgRx.data[i];
                return TRUE;
            }
        }
    }
    can.reset();
    CANTimer.stop();
    return FALSE;
}


// gmlan.h - information and definitions needed for communicating with GMLAN

// (C) 2013 Sophie Dexter

#ifndef __GMLAN_H__
#define __GMLAN_H__

#include "mbed.h"

#include "common.h"
#include "canutils.h"

#define T8REQID 0x7E0
#define T8RESPID 0x7E8

#define T8USDTREQID 0x011
#define T8UUDTRESPID 0x311
#define T8USDTRESPID 0x411

#define GMLANALLNODES 0x101

#define GMLANMESSAGETIMEOUT 50             // 50 milliseconds (0.05 of a second) - Seems to be plenty of time to wait for messages on the CAN bus
#define GMLANPTCT 150                       // 150 milliseconds Timeout between tester request and ECU response or multiple ECU responses
#define GMLANPTCTENHANCED 5100                       // 5100 milliseconds Enhanced response timing value for tester P2CT timer.

// Tester Present message, must be sent at least once every 5 seconds for some types of activity
// NOTE however that 2 seconds has been suggested as a good interval
#define GMLANTesterPresentPhysical    {0x01,0x3E,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
#define GMLANTesterPresentFunctional  {0xFE,0x01,0x3E,0xaa,0xaa,0xaa,0xaa,0xaa}
extern void GMLANTesterPresentAll();
extern void GMLANTesterPresent(uint32_t ReqID, uint32_t RespID);

// All steps needed in preparation for using a bootloader ('Utility File' in GMLAN parlance)
bool GMLANprogrammingSetupProcess(uint32_t ReqID, uint32_t RespID);

// All steps needed to transfer and start a bootloader ('Utility File' in GMLAN parlance)
bool GMLANprogrammingUtilityFileProcess(uint32_t ReqID, uint32_t RespID, const uint8_t UtilityFile[]);


// Start a Diagnostic Session
#define GMLANinitiateDiagnosticOperation {0x02,0x10,0x02,0xaa,0xaa,0xaa,0xaa,0xaa}
#define GMLANdisableAllDTCs 0x02
bool GMLANinitiateDiagnostic(uint32_t ReqID, uint32_t RespID, char level);


// Tell T8 To disable normal communication messages
#define GMLANdisableCommunication {0x01,0x28,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
bool GMLANdisableNormalCommunication(uint32_t ReqID, uint32_t RespID);


// Tell T8 To report programmed state
#define GMLANReportProgrammed    {0x01,0xA2,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
bool GMLANReportProgrammedState(uint32_t ReqID, uint32_t RespID);

// Tell T8 To request or initiate programming
#define GMLANProgramming    {0x02,0xA5,0x00,0xaa,0xaa,0xaa,0xaa,0xaa}
#define GMLANRequestProgrammingNormal 0x01  // I or P-Bus speed
#define GMLANRequestProgrammingFast 0x02    // 83.333 kbps for single-wire CAN
#define GMLANEnableProgrammingMode 0x03     // Tell T8 To initiate programming
bool GMLANProgrammingMode(uint32_t ReqID, uint32_t RespID, char mode);

// authenticate with T8
#define GMLANSecurityAccessSeed     {0x02,0x27,0x01,0xaa,0xaa,0xaa,0xaa,0xaa}
bool GMLANSecurityAccessRequest(uint32_t ReqID, uint32_t RespID, char level, uint16_t& seed);
#define GMLANSecurityAccessKey      {0x04,0x27,0x02,0xCA,0xFE,0xaa,0xaa,0xaa}
bool GMLANSecurityAccessSendKey(uint32_t ReqID, uint32_t RespID, char level, uint16_t key);

// Tell T8 We are Requesting a download session
#define GMLANRequestDownloadMessage {0x06,0x34,0x00,0x00,0x00,0x00,0x00,0xaa}
#define GMLANRequestDownloadModeNormal 0x00
#define GMLANRequestDownloadModeEncrypted 0x01
#define GMLANRequestDownloadModeCompressed 0x10
#define GMLANRequestDownloadModeCompressedEncrypted 0x11
bool GMLANRequestDownload(uint32_t ReqID, uint32_t RespID, char dataFormatIdentifier);

// Data blocks are sent using this message type
#define GMLANDataTransferMessage    {0x10,0xF0,0x36,0x00,0xCA,0xFE,0xBA,0xBE}
#define GMLANDOWNLOAD 0x00
#define GMLANEXECUTE  0x80
bool GMLANDataTransfer(uint32_t ReqID, uint32_t RespID, char length, char function, uint32_t address);
bool GMLANDataTransferFirstFrame(uint32_t ReqID, uint32_t RespID, char length, char function, uint32_t address);
bool GMLANDataTransferConsecutiveFrame(uint32_t ReqID, char framenumber, char data[7]);
bool GMLANDataTransferBlockAcknowledge(uint32_t RespID);

// Tell T8 ECU to return to normal mode after FLASHing
#define GMLANReturnToNormalModeMessage    {0x01,0x20,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
bool GMLANReturnToNormalMode(uint32_t ReqID, uint32_t RespID);


// Show a description of GMLAN Return Codes when an error occurs
void GMLANShowReturnCode(char returnCode);

#endif
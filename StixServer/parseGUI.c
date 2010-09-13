#include "parseGUI.h"

GUIHandler const handlers[] = {
        NULL, // 0x00 Nothing defined here, move along
        NULL, // 0x01 STM
        NULL, // 0x02 RTM
        NULL, // 0x03 RSD
        NULL, // 0x04 RSF
        NULL, // 0x05 DSF
        NULL, // 0x06 PMW
        NULL, // 0x07 PMC
        NULL, // 0x08 HTP
        NULL, // 0x09 HTC
        NULL, // 0x0A LTC
        NULL, // 0x0B SVS
        receiveParameterBytes, // 0x0C SSP
        receiveWaveSetCommand, // 0x0D SCP
        sendSpecSample, // 0x0E RFS
        sendCalCos, // 0x0F RCD
        sendSpecParameters, // 0x10 RSP
        sendWaveParameters, // 0x11 RCP
        writeConfigChanges, // 0x12 SVC
        sendAbsorbance, // 0x13 RAD
        NULL, // 0x14 RCC
        receiveRecordDarkSample, // 0x15 RDS
        receiveRecordRefSample, // 0x16 RRS
        receiveRecordSpecSample, // 0x17 RSS
        NULL, // 0x18 EXM
        NULL, // 0x19 TRM
        receiveMethodFile, // 0x1A LRM
        sendMethodFileList, // 0x1B LMT
        sendMethodFile, // 0x1C RMF
        receiveSetCurrentMethodFile, // 0x1D SMF
        receiveDeleteMethodFile, // 0x1E DLM
        sendAbsorbanceSpectrum, // 0x1F RAS
        sendActiveMethodFile // 0x20 SAM
    };

static const int numHandlers = 33;

void parseGUI(int connection,char* command){
    if(command[0] < numHandlers && command[0] > 0){
        if(handlers[command[0]]){
            handlers[command[0]](connection,command);
        } else {
            syslog(LOG_DAEMON||LOG_INFO,"Nothing to do, function handlers for %x not yet implemented.", command[0]);
        }
    } else {
        syslog(LOG_DAEMON||LOG_ERR,"Command Invalid.  %x is out of the range of possible commands.", command[0]);
    }
}


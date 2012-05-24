#include "parseGUI.h"

GUIHandler const handlers[] = {
        NULL, // 0x00 Nothing defined here, move along
        NULL, // 0x01 STM
        NULL, // 0x02 RTM
        NULL, // 0x03 RSD
        NULL, // 0x04 RSF
        NULL, // 0x05 DSF
        receiveSetPumpPercent, // 0x06 PMW
        receiveSetPumpControl, // 0x07 PMC
        receiveSetHeaterTemp, // 0x08 HTP
        receiveSetHeaterControl, // 0x09 HTC
        receiveSetLampControl, // 0x0A LTC
        receiveSlopeInterceptPairs, // 0x0B LSD 
        receiveParameterBytes, // 0x0C SSP
        receiveWaveSetCommand, // 0x0D SCP
        sendSpecSample, // 0x0E RFS
        sendCalCos, // 0x0F RCD
        sendSpecParameters, // 0x10 RSP
        sendWaveParameters, // 0x11 RCP
        writeConfigChanges, // 0x12 SVC
        sendAbsorbance, // 0x13 RAD
        sendConcentration, // 0x14 RCC
        receiveRecordDarkSample, // 0x15 RDS
        receiveRecordRefSample, // 0x16 RRS
        receiveRecordSpecSample, // 0x17 RSS
        receiveExecuteMethod, // 0x18 EXM
        receiveTerminateMethod, // 0x19 TRM
        receiveMethodFile, // 0x1A LRM
        sendMethodFileList, // 0x1B LMT
        sendMethodFile, // 0x1C RMF
        receiveSetCurrentMethodFile, // 0x1D SMF
        receiveDeleteMethodFile, // 0x1E DLM
        sendAbsorbanceSpectrum, // 0x1F RAS
        sendActiveMethodFile, // 0x20 SAM
        receiveGetPumpStatus, // 0x21 RPS
        receiveGetHeaterStatus, // 0x22 RHS
        receiveGetLampStatus, // 0x23 RLS
        receiveGetBatteryVoltage, // 0x24 RBS
        receiveGetCTDValues, // 0x25 RTD
        receiveGetTemperatureValue, // 0x26 RHT
        receiveSaveBenchConfig, // 0x27 SBC
        sendBenchConfig // 0x28 RBC
    };

static const int numGUIHandlers = 0x28;

void parseGUI(int connection,char* command){
    char ack = 0x00;
    if(command[0] <= numGUIHandlers && command[0] > 0){
        if(handlers[command[0]]){
            send(connection,&ack,1,0);
            handlers[command[0]](connection,command);
        } else {
            syslog(LOG_DAEMON||LOG_INFO,"Nothing to do, function handlers for %x not yet implemented.", command[0]);
        }
    } else {
        syslog(LOG_DAEMON||LOG_ERR,"Command Invalid.  %x is out of the range of possible commands.", command[0]);
    }
}

void sendErrorMessageBack(int connection, char* message){
    char* completeMessage;
    unsigned int messageLength = strlen(message) + 2;

    completeMessage = (char*)malloc(sizeof(char)*messageLength);
    completeMessage[0] = ERR;
    completeMessage[1] = '\0';
    strcat(completeMessage+1,message);
    send(connection,completeMessage,messageLength,0);
    free(completeMessage);
}

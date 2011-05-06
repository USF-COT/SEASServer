#include "runProtocolManager.h"

void pumpRunResponse(int connection, s_node* node){
    PUMP_ON_RUNTIME_DATA onData;
    PUMP_OFF_RUNTIME_DATA offData;
    
    double* argv = (double*)node->argv;

    if(node->commandID == PUMP_ON_RUNTIME_CMD){
        onData.Header.HeadByte = RTH;
        onData.Header.Command = node->commandID;
        onData.Pump = (uint16_t) argv[0];
        onData.Percent = (uint16_t) argv[1];
        send(connection,&onData,sizeof(PUMP_ON_RUNTIME_DATA),0);
    } else if (node->commandID == PUMP_OFF_RUNTIME_CMD) {
        offData.Header.HeadByte = RTH;
        offData.Header.Command = node->commandID;
        offData.Pump = (uint16_t) argv[0];
        send(connection,&offData,sizeof(PUMP_OFF_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to pumpRunResponse.",node->commandID);
    }
}

void lampRunResponse(int connection, s_node* node){
    RUNTIME_RESPONSE_HEADER record;

    if(node->commandID == LAMP_ON_RUNTIME_CMD || node->commandID == LAMP_OFF_RUNTIME_CMD){
        record.HeadByte = RTH;
        record.Command = node->commandID;
        send(connection,(void*)&record,sizeof(RUNTIME_RESPONSE_HEADER),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to lampRunResponse.",node->commandID);
    }
}

void heaterRunResponse(int connection, s_node* node){
    HEATER_ON_RUNTIME_DATA heatOnData;
    RUNTIME_RESPONSE_HEADER heatOffData;

    if(node->commandID == HEATER_ON_RUNTIME_CMD){
        heatOnData.Header.HeadByte = RTH;
        heatOnData.Header.Command = node->commandID;
        heatOnData.Temperature = (float)((double*)node->argv)[0];
        send(connection,(void*)&heatOnData,sizeof(HEATER_ON_RUNTIME_DATA),0);
    } else if(node->commandID == HEATER_OFF_RUNTIME_CMD){
        heatOffData.HeadByte = RTH;
        heatOffData.Command = node->commandID; 
        send(connection,(void*)&heatOffData,sizeof(RUNTIME_RESPONSE_HEADER),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to heaterRunResponse.",node->commandID);
    }
}

// All Spectrometer Run Methods are in USB4000Manager.h/c Files

void delayRunResponse(int connection, s_node* node){
    DELAY_RUNTIME_DATA data;

    if(node->commandID == DELAY_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.SecondsRemaining = (uint16_t)node->argc;
        send(connection,(void*)&data,sizeof(DELAY_RUNTIME_DATA),0);
        syslog(LOG_DAEMON|LOG_INFO,"Delayed for a second.  %d seconds remaining.",data.SecondsRemaining);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to delayRunResponse.",node->commandID);
    }
}

void waitHeaterRunResponse(int connection, s_node* node){
    WAIT_HEATER_RUNTIME_DATA data;
    unsigned long* args = (unsigned long*)node->argv;

    if(node->commandID == WAIT_HEATER_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        
        // Check to see if the counter has just been reset
        if(node->argc ==  args[0]){
            data.Seconds = 0xFFFF;
        } else {
            data.Seconds = node->argc;
        }

        // Get the heater status to report the current temperature back
        data.Temperature = getHeaterCurrentTemperature(1);
        send(connection,(void*)&data,sizeof(WAIT_HEATER_RUNTIME_DATA),0);
        syslog(LOG_DAEMON|LOG_INFO,"Sent wait heater command run response.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to waitHeaterRunResponse.", node->commandID);
    }
}

void readTempRunResponse(int connection, s_node* node){
    READ_TEMPERATURE_RUNTIME_DATA data;

    if(node->commandID == READ_TEMPERATURE_RUNTIME_CMD){
        data.Header.Command = node->commandID;
        data.Temperature = getHeaterCurrentTemperature(1);
        send(connection,(void*)&data,sizeof(READ_TEMPERATURE_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to readTempResponse.",node->commandID);
    }
}

void setDwellRunResponse(int connection, s_node* node){
    SET_DWELL_RUNTIME_DATA data;
    double* argv = (double*)node->argv;

    if(node->commandID == SET_DWELL_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (unsigned char) argv[0]-1;
        data.Seconds = (uint16_t) getDwell(data.Spectrometer);
        send(connection,(void*)&data,sizeof(SET_DWELL_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unknown command ID %02x passed to setDwellRunResponse.",node->commandID);
    }
}

RUNNODEHandler const runProtoHandlers[] = {
    pumpRunResponse,
    pumpRunResponse,
    lampRunResponse,
    lampRunResponse,
    heaterRunResponse,
    heaterRunResponse,
    readRefRunResponse,
    readSampRunResponse,
    calConcRunResponse,
    calPCO2RunResponse,
    calPHRunResponse,
    calTCRunResponse,
    readFullSpecRunResponse,
    delayRunResponse,
    waitHeaterRunResponse,
    readTempRunResponse,
    setDwellRunResponse,
    NULL
};

void sendRunProtocolMessage(int connection, s_node* node){
    if(node->commandID < MAX_RUNTIME_COMMANDS-1){
         if(runProtoHandlers[node->commandID]){
             runProtoHandlers[node->commandID](connection,node);
         } else {
             syslog(LOG_DAEMON|LOG_INFO,"No handler defined for command ID %02x.",node->commandID);
         }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Command ID %02x is out of range (%02x to %02x)",node->commandID,0,MAX_RUNTIME_COMMANDS-1);
    }    
}


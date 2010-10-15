#include "SEASPeripheralCommands.h"

// Base Functions
void pumpOn(unsigned char pumpID){
    LONresponse_s* response;
    unsigned char data[2] = {pumpID,ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Enabling pump %d.",pumpID);
    response = sendLONCommand(PMP,PWR,2,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d enabled.  LON sent ACK.",pumpID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR, "ERROR: Pump %d not enabled.  LON sent NAK.",pumpID);
    }
    freeLONResponse(response);
}
void pumpOff(unsigned char pumpID){
    LONresponse_s* response;
    unsigned char data[2] = {pumpID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Disabling pump %d.",pumpID);
    response = sendLONCommand(PMP,PWR,2,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d disabled.  LON sent ACK.",pumpID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d not diabled.  LON sent NAK.",pumpID);
    }
    freeLONResponse(response);
}

void setPumpRPM(unsigned char pumpID,unsigned int RPM){
    LONresponse_s* response;
    unsigned char data[3];

    data[0] = pumpID;
    data[1] = (unsigned char)((RPM & 0xFF00) >> 8);
    data[2] = (unsigned char)(RPM & 0xFF);

    syslog(LOG_DAEMON|LOG_INFO,"Setting pump %d RPM to: %d.",pumpID,RPM);
    response = sendLONCommand(PMP,PWL,3,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d RPM set to: %d.",pumpID,RPM);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d RPM NOT set to: %d.",pumpID,RPM);
    }
    freeLONResponse(response);
}

void lampOn(){
    LONresponse_s* response;
    unsigned char data[1] = {ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp on.");    
    response = sendLONCommand(LTE,PWR,1,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned on.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned on.");
    }
    freeLONResponse(response);
}

void lampOff(){
    LONresponse_s* response;
    unsigned char data[1] = {DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp off.");
    response = sendLONCommand(LTE,PWR,1,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned off.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned off.");
    }
    freeLONResponse(response);
}

void heaterOn(unsigned char heaterID){
    LONresponse_s* response;
    unsigned char data[2] = {heaterID,ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Turning on Heater %d.",heaterID);
    response = sendLONCommand(HTR,PWR,2,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned on.",heaterID); 
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned on.",heaterID);
    }
    freeLONResponse(response);
}

void heaterOff(unsigned char heaterID){
    LONresponse_s* response;
    unsigned char data[2] = {heaterID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Turning off Heater %d.",heaterID);
    response = sendLONCommand(HTR,PWR,2,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned off.",heaterID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned off.",heaterID);
    }
    freeLONResponse(response);
}

void setHeaterTemp(unsigned char heaterID, float temperature){
    LONresponse_s* response;
    unsigned char data[5];

    data[0] = heaterID;
    memcpy(data+1,&temperature,4);
    
    syslog(LOG_DAEMON|LOG_INFO,"Setting heater %d temperature to: %f.",heaterID,temperature);
    response = sendLONCommand(HTR,TMP,5,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d temperature set to: %f.",heaterID,temperature);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d temperature NOT set to: %f.",heaterID,temperature);
    }
    freeLONResponse(response);
}

// Status Functions

pumpStatus_s* getPumpStatus(unsigned char pumpID){
    pumpStatus_s* status = NULL;
    unsigned char sendData[1] = {pumpID};
    LONresponse_s* response = sendLONCommand(PMP,STS,1,sendData);
    if(response){
        if(response->data && response->deviceID == PMP){
            status = malloc(sizeof(pumpStatus_s));
            status->pumpID = response->data[0];
            status->power = response->data[1];
            memcpy(&(status->RPM),response->data+2,4);        
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to retrieve pump %d status.",pumpID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Did not receive a response from LON for pump %d status command.",pumpID);
    }
    return status;
}

float getHeaterCurrentTemperature(unsigned char heaterID){
    float currentTemp = -1;
    unsigned char sendData[1] = {heaterID};
    LONresponse_s* response = sendLONCommand(HTR,RCT,1,sendData);
    if(response){
        if(response->data && response->deviceID == HTR && response->data[0] == heaterID){
            memcpy(&currentTemp,response->data+1,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to retrieve current temperature from heater %d.",heaterID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for current temperature from heater %d.",heaterID);
    }
    return currentTemp;
}

heaterStatus_s* getHeaterStatus(unsigned char heaterID){
    heaterStatus_s* status = NULL;
    unsigned char sendData[1] = {heaterID};
    LONresponse_s* response = sendLONCommand(HTR,STS,1,sendData);
    if(response){
        if(response->data && response->deviceID == HTR && response->data[0] == heaterID){
            status = malloc(sizeof(heaterStatus_s));
            status->heaterID = heaterID;
            status->power = response->data[1];
            memcpy(&(status->setTemperature),response->data+2,4);
            memcpy(&(status->currentTemperature),response->data+4,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for status of heater %d.",heaterID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for status from heater %d",heaterID);
    }
    return status;
}

unsigned char getLampStatus(){
    unsigned char status = DIS;
    LONresponse_s* response = sendLONCommand(LTE,STS,0,NULL);
    if(response){
        if(response->data && response->deviceID == LTE){
            status = response->data[0];
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for status of lamp.");
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for status of lamp.");
    }
    return status;
}

float getBatteryVoltage(){
    float voltage = 0;
    LONresponse_s* response = sendLONCommand(BAT,BVR,0,NULL);
    if(response){
        if(response->data && response->deviceID == BAT){
            memcpy(&voltage,response->data,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for battery voltage.");
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for battery voltage.");
    }
    return voltage;
}

CTDreadings_s* getCTDValues(){
    CTDreadings_s* readings = NULL;
    LONresponse_s* response = sendLONCommand(CTD,CTR,0,NULL);
    if(response){
        if(response->data && response->deviceID == CTD){
            readings = malloc(sizeof(CTDreadings_s));
            memcpy(&(readings->conductivity),response->data,4);
            memcpy(&(readings->temperature),response->data+4,4);
            memcpy(&(readings->pressure),response->data+8,4);
            memcpy(&(readings->soundVelocity),response->data+12,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for CTD readings.");
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for battery voltage.");
    }
    return readings;
}

// GUI Protocol Wrappers
void receiveSetPumpControl(int connection, char* command){
    if(command[0] == PMC){
        syslog(LOG_DAEMON|LOG_INFO,"Switching pump %d %s.",command[3],command[4] == 1 ? "ON" : "OFF");
        if(command[4] == 1){
            pumpOn(command[3]);
        } else {
            pumpOff(command[3]);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetPumpControl method.");
    }
}

void receiveSetPumpRPM(int connection, char* command){
    unsigned char pumpID;
    unsigned int RPM;

    if(command[0] == PMW){
        pumpID = command[3];
        RPM = (command[4] << 8) + command[5];
        syslog(LOG_DAEMON|LOG_INFO,"Setting pump %d Power to: %d",pumpID,RPM);
        setPumpRPM(pumpID,RPM);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetPumpRPM method.");
    }
}

void receiveSetLampControl(int connection, char* command){
    if(command[0] == LTC){
        syslog(LOG_DAEMON|LOG_INFO,"Turning lamp %s.",command[3] == 1 ? "ON" : "OFF");
        if(command[3] == 1){
            lampOn();
        } else {
            lampOff();
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetLampControl method.");
    }
}

void receiveSetHeaterControl(int connection, char* command){
    unsigned char heaterID;

    if(command[0] == HTC){
        heaterID = command[3];
        syslog(LOG_DAEMON|LOG_INFO,"Turning heater %d %s.",heaterID,command[4] == 1 ? "ON" : "OFF");
        if(command[4] == 1){
            heaterOn(heaterID);
        } else {
            heaterOff(heaterID);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetHeaterControl method.");
    }
}

void receiveSetHeaterTemp(int connection, char* command){
    unsigned char heaterID;
    float temperature;

    if(command[0] == HTP){
        heaterID = command[3];
        memcpy(&temperature,command+4,4);
        syslog(LOG_DAEMON|LOG_INFO,"Setting heater %d temperature to %f.",heaterID,temperature);
        setHeaterTemp(heaterID,temperature);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetHeaterControl method.");
    }
}


void receiveGetPumpStatus(int connection, char* command){
    pumpStatus_s* status = NULL;
    unsigned char sendBuffer[9];
    syslog(LOG_DAEMON|LOG_INFO,"Getting pump %d status.",command[3]);

    if(command[0] == RPS){
        status = getPumpStatus(command[3]);
        if(status){
            syslog(LOG_DAEMON|LOG_INFO,"Pump %d status retrieved.  Pump is %s. RPM is %d.",status->pumpID,status->power == ENA ? "ON" : "OFF",status->RPM);
            sendBuffer[0] = RPS;
            sendBuffer[1] = 0;
            sendBuffer[2] = 7;
            sendBuffer[3] = status->pumpID;
            sendBuffer[4] = status->power == ENA;
            memcpy(sendBuffer+5,&(status->RPM),4);
            send(connection,sendBuffer,9,0);
            free(status);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d status not retrieved from LON.",command[3]);
            sendErrorMessageBack(connection,"ERROR: Pump status not retrieved from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetPumpStatus method.");
    }
}

void receiveGetHeaterStatus(int connection, char* command){
    heaterStatus_s* status = NULL;
    unsigned char sendBuffer[13];
    syslog(LOG_DAEMON|LOG_INFO,"Getting heater %d status.",command[3]);
    
    if(command[0] == RHS){
        status = getHeaterStatus(command[3]);
        if(status){
            syslog(LOG_DAEMON|LOG_INFO,"Heater %d status retrieved.  Heater is %s.  Set temperature is %f.  Current temperature is %f.",status->heaterID,status->power == ENA ? "ON" : "OFF",status->setTemperature,status->currentTemperature);
            sendBuffer[0] = RHS;
            sendBuffer[1] = 0;
            sendBuffer[2] = 13;
            sendBuffer[3] = status->heaterID;
            sendBuffer[4] = status->power == ENA;
            memcpy(sendBuffer+5,&(status->setTemperature),4);
            memcpy(sendBuffer+9,&(status->currentTemperature),4);
            send(connection,sendBuffer,13,0);
            free(status);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d status not retrieved from LON.",command[3]);
            sendErrorMessageBack(connection,"ERROR: Heater status not retrieved from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetHeaterStatus method.");
    }
}

void receiveGetLampStatus(int connection, char* command){
    unsigned char sendBuffer[4];

    syslog(LOG_DAEMON|LOG_INFO,"Getting light status.");
    if(command[0] == RLS){
        sendBuffer[0] = RLS;
        sendBuffer[1] = 0;
        sendBuffer[2] = 4;
        sendBuffer[3] = getLampStatus() == ENA;
        send(connection,sendBuffer,4,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetLampStatus method.");
    }
}

void receiveGetBatteryVoltage(int connection, char* command){
    unsigned char sendBuffer[7];
    float batteryVoltage;

    syslog(LOG_DAEMON|LOG_INFO,"Getting battery voltage.");
    if(command[0] == RBS){
        batteryVoltage = getBatteryVoltage();
        syslog(LOG_DAEMON|LOG_INFO,"Retrieved battery voltage as: %f.",batteryVoltage);
        sendBuffer[0] = RBS;
        sendBuffer[1] = 0;
        sendBuffer[2] = 7;
        memcpy(sendBuffer+3,&batteryVoltage,4);
        send(connection,sendBuffer,4,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveBatteryVoltage method.");
    }
}

void receiveGetCTDValues(int connection, char* command){
    unsigned char sendBuffer[19];
    CTDreadings_s* readings = NULL;

    if(command[0] == RTD){
        readings = getCTDValues();
        if(readings){
            syslog(LOG_DAEMON|LOG_INFO,"CTD Readings - Cond: %f, Temp: %f, Pres: %f, Sound: %f.",readings->conductivity,readings->temperature,readings->pressure,readings->soundVelocity);
            sendBuffer[0] = RTD;
            sendBuffer[1] = 0;
            sendBuffer[2] = 19;
            memcpy(sendBuffer+3,&(readings->conductivity),4);
            memcpy(sendBuffer+7,&(readings->temperature),4);
            memcpy(sendBuffer+11,&(readings->pressure),4);
            memcpy(sendBuffer+15,&(readings->soundVelocity),4);
            send(connection,sendBuffer,19,0);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Could not read CTD readings from LON.");
            sendErrorMessageBack(connection,"ERROR: Could not read CTD readings from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetCTDValues method.");
    }
}

// Method Wrappers
void methodPumpOn(unsigned long argc, void* argv){
    unsigned char pumpID;
    unsigned int RPM;
    double* arguments = (double*) argv;

    if(argc != 2){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to methodPumpOn function.");
        return;
    }

    pumpID = (unsigned char) arguments[0];
    RPM = (unsigned int) arguments[1];

    pumpOn(pumpID);
    setPumpRPM(pumpID,RPM);
}

void methodPumpOff(unsigned long argc, void* argv){
    unsigned char pumpID;
    double* arguments = (double*) argv;

    if(argc != 1){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to methodPumpOff function.");
        return;
    }

    pumpID = (unsigned char) arguments[0];
    pumpOff(pumpID);
}

void methodLampOn(unsigned long argc, void* argv){
    if(argc != 0){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to lampOn function.");
        return;
    }
    lampOn();
}

void methodLampOff(unsigned long argc, void* argv){
    if(argc != 0){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to lampOff function.");
        return;
    }
    lampOff();
}

void methodHeaterOn(unsigned long argc, void* argv){
    unsigned char heaterID;
    float temperature;
    double* arguments = (double*)argv;

    if(argc != 2){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to heaterOn function.");
        return;
    }

    heaterID = (unsigned char)arguments[0];
    temperature = (float)arguments[1];
    heaterOn(heaterID);
    setHeaterTemp(heaterID,temperature);
}

void methodHeaterOff(unsigned long argc, void* argv){
    unsigned char heaterID;
    double* arguments = (double*)argv;

    if(argc != 1){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to heaterOff function.");
        return;
    }

    heaterID = (unsigned char)arguments[0];
    heaterOff(heaterID);
}


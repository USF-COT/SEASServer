#include "SEASPeripheralCommands.h"

// Base Functions
void pumpOn(unsigned char pumpID){
    unsigned char* response;
    unsigned char data[2] = {pumpID,ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Enabling pump %d.",pumpID);
    response = sendLONCommand(PMP,PWR,2,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d enabled.  LON sent ACK.",pumpID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR, "ERROR: Pump %d not enabled.  LON sent NAK.",pumpID);
    }
    free(response);
}
void pumpOff(unsigned char pumpID){
    unsigned char* response;
    unsigned char data[2] = {pumpID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Disabling pump %d.",pumpID);
    response = sendLONCommand(PMP,PWR,2,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d disabled.  LON sent ACK.",pumpID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d not diabled.  LON sent NAK.",pumpID);
    }
    free(response);
}

void setPumpRPM(unsigned char pumpID,unsigned int RPM){
    unsigned char* response;
    unsigned char data[3];

    data[0] = pumpID;
    data[1] = (unsigned char)((RPM & 0xFF00) >> 8);
    data[2] = (unsigned char)(RPM & 0xFF);

    syslog(LOG_DAEMON|LOG_INFO,"Setting pump %d RPM to: %d.",pumpID,RPM);
    response = sendLONCommand(PMP,PWL,3,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d RPM set to: %d.",pumpID,RPM);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d RPM NOT set to: %d.",pumpID,RPM);
    }
    free(response);
}

void lampOn(){
    unsigned char* response;
    unsigned char data[1] = {ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp on.");    
    response = sendLONCommand(LTE,PWR,1,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned on.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned on.");
    }
    free(response);
}

void lampOff(){
    unsigned char* response;
    unsigned char data[1] = {DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp off.");
    response = sendLONCommand(LTE,PWR,1,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned off.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned off.");
    }
    free(response);
}

void heaterOn(unsigned char heaterID){
    unsigned char* response;
    unsigned char data[2] = {heaterID,ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Turning on Heater %d.",heaterID);
    response = sendLONCommand(HTR,PWR,2,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned on.",heaterID); 
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned on.",heaterID);
    }
    free(response);
}

void heaterOff(unsigned char heaterID){
    unsigned char* response;
    unsigned char data[2] = {heaterID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Turning off Heater %d.",heaterID);
    response = sendLONCommand(HTR,PWR,2,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned off.",heaterID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned off.",heaterID);
    }
    free(response);
}

void setHeaterTemp(unsigned char heaterID, float temperature){
    unsigned char* response;
    unsigned char data[5];

    data[0] = heaterID;
    memcpy(data+1,&temperature,4);
    
    syslog(LOG_DAEMON|LOG_INFO,"Setting heater %d temperature to: %f.",heaterID,temperature);
    response = sendLONCommand(HTR,TMP,5,data);
    if(response[0] == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d temperature set to: %f.",heaterID,temperature);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d temperature NOT set to: %f.",heaterID,temperature);
    }
    free(response);
}

// GUI Protocol Wrappers

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


#include "configManager.h"

void receiveWaveSetCommand(int connection, char* command){
    int specID;
    char newAnaName[MAX_ANA_NAME];
    unsigned char newUnits;
    unsigned char absWaveCount;
    float absWaves[9];
    float nonAbsWave;

    syslog(LOG_DAEMON||LOG_INFO,"Setting Wavelength Configuration."); 
    specID = (int)command[1];

    setComputationDataBytes(specID,command+2);
    syslog(LOG_DAEMON||LOG_INFO,"Wavelength Configuration Set.");
}

void sendWaveParameters(int connection, char* command){
    wavelengthParameters* params = NULL;

    syslog(LOG_DAEMON||LOG_INFO,"Sending wavelength parameters.");
    params = getWaveParameters(command[1]);
    if(params){
        send(connection,(void *)params,sizeof(wavelengthParameters),0);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Wavelength parameters sent.");
}

void receiveParameterBytes(int connection, char* command){
    int specID;
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;

    syslog(LOG_DAEMON||LOG_INFO,"Setting Spectrometer Parameters...");
    specID = (int)command[1];
    memcpy(&integrationTime,command+2,2);
    memcpy(&scansPerSample,command+4,2);
    memcpy(&boxcarSmoothing,command+6,2);

    setSpectrometerParameters(specID,integrationTime,scansPerSample,boxcarSmoothing);
    syslog(LOG_DAEMON||LOG_INFO,"Spectrometer Parameters Set.");
}

void sendSpecParameters(int connection, char* command){
    spectrometerParameters* params = NULL;

    syslog(LOG_DAEMON||LOG_INFO,"Sending spectrometer %d parameters.",command[1]);
    params = getSpecParameters(command[1]);
    if(params){
        send(connection,(void*)params,sizeof(spectrometerParameters),0);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Spectrometer %d parameters sent.",command[1]);
}

void writeConfigChanges(int connection, char* command){
    syslog(LOG_DAEMON||LOG_INFO,"Saving Configuration.");
    char response[1] = {SVC};
    writeConfigFile();
    
    send(connection,response,sizeof(char),0);
    syslog(LOG_DAEMON||LOG_INFO,"Configuration Saved.");
}

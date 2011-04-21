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

void receiveSlopeInterceptPairs(int connection, char* command){
    int i=0;

    if(command[0] == LSD){
        syslog(LOG_DAEMON|LOG_INFO,"Setting Slope Intercept Pairs for Spectrometers.");
        for(i=0; i < 2; i++){
            setSlopeIntercept(i,(float*)command+(1+i*MAX_ABS_WAVES));
        }
        syslog(LOG_DAEMON|LOG_INFO,"Successfully Set Slope Intercept Pairs.");
    } else {
        syslog(LOG_DAEMON|LOG_INFO,"Unrecognized Command (%02X) Passed to Slope Intercept Pair Receiver.",command[0]);
        sendErrorMessageBack(connection,"Unrecognized Command Sent to Slope Intercept Pair Receiver.");
    }
}

void writeConfigChanges(int connection, char* command){
    syslog(LOG_DAEMON||LOG_INFO,"Saving Configuration.");
    char response[1] = {SVC};
    writeConfigFile();
    
    send(connection,response,sizeof(char),0);
    syslog(LOG_DAEMON||LOG_INFO,"Configuration Saved.");
}

// Method Script Handlers
void methodSetSpectrometerParameters(unsigned long argc, void* argv){
    double* params = (double*)argv;
    int specIndex;
    unsigned short intTime,scansPerSamp,boxcarSmooth;

    specIndex = (int)params[0]-1;
    intTime = (unsigned short)params[1];
    scansPerSamp = (unsigned short)params[2];
    boxcarSmooth = (unsigned short)params[3];
    syslog(LOG_DAEMON|LOG_INFO,"Setting spectrometer %d parameters.  Integration Time:%d,Scans Per Sample:%d,Boxcar Smoothing:%d.",specIndex,intTime,scansPerSamp,boxcarSmooth);

    setSpectrometerParameters(specIndex,intTime,scansPerSamp,boxcarSmooth);
    applyConfig();
    writeConfigFile();
}

void methodSetAbsorbanceWavelengths(unsigned long argc, void* argv){
    double* params = (double*)argv;
    int specIndex,i;
    unsigned char absWaveCount;
    float* waves;

    // This is a special circumstance because the number of wavelengths is variable
    absWaveCount = ((unsigned char)params[0])-2; // Array contents [Array Length] [Spec ID] [Wavelength]...[Wavelength]
    waves = malloc(sizeof(float)*absWaveCount);
    specIndex = (int)params[1]-1;
    syslog(LOG_DAEMON|LOG_INFO,"Setting %d wavelengths for spectrometer %d.",absWaveCount,specIndex);
    for(i=0; i < absWaveCount; i++){
        waves[i] = (float)params[i+2];
        syslog(LOG_DAEMON|LOG_INFO,"Wave %d:%f",i,waves[i]);
    }
    setAbsorbanceWavelengths(specIndex,absWaveCount,waves);
    free(waves); 
}

void methodSetNonAbsorbanceWavelength(unsigned long argc, void* argv){
    double* params = (double*)argv;
    int specIndex;
    float nonAbsWave;
     
    specIndex = (int)params[0]-1;
    nonAbsWave = (float)params[1];
    syslog(LOG_DAEMON|LOG_INFO,"Setting spectrometer %d non-absorbing wavelength to %f.",specIndex,nonAbsWave);

    setNonAbsorbingWavelengths(specIndex,nonAbsWave);
}

void methodSetDwell(unsigned long argc, void* argv){
    double* params = (double*)argv;
    int specIndex;
    int dwell;

    specIndex = (int)params[0]-1;
    dwell = (int)params[1];
    syslog(LOG_DAEMON|LOG_INFO,"Setting spectrometer %d dwell to %d.",specIndex,dwell);

    setDwell(specIndex,dwell);
}

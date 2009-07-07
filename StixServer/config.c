#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslog.h>

#define CONFIGPATH "/media/card/StixServerConfig.txt"
#define MAXCONFIGLINE 128

spectrometerParameters parameters[2];

char readConfig(){

    FILE* configFile;
    char configLine[MAXCONFIGLINE];
    char* tok;
    int specIndex,wavelengthCount;

    if(access(CONFIGPATH,R_OK||W_OK)){
        return 0; // Do not have access to file
    }

    configFile = fopen(CONFIGPATH,"r+");
    while(!feof(configFile)){
        fgets(configLine,MAXCONFIGLINE,configFile);
 
        // Skip Comments and Empty Lines
        if(configLine[0] == '#' || strlen(configLine) == 0){
            syslog(LOG_DAEMON||LOG_INFO,"Skipped Commented or Empty Config Line: %s",configLine);
            continue;
        }
        else{
            tok = strtok(configLine,".");
            if(strcmp(tok,"SPEC1") == 0){
                specIndex = 0;
            }
            else if(strcmp(tok,"SPEC2") == 0){
                specIndex = 1;
            }
            else{ // Unrecognized prefix, skip this line.
                syslog(LOG_DAEMON||LOG_INFO,"Skipped Unrecognized Prefixed Config Line: %s",configLine);
                continue;
            }
            
            tok = strtok(NULL,"="); 
            if(strcmp(tok,"SERIAL") == 0){
                tok = strtok(NULL,"\n\d");
                strncpy(parameters[specIndex].serial,tok,11);
            }
            else if(strcmp(tok,"INTEGRATION_TIME") == 0){
                tok = strtok(NULL,"\n\d");
                parameters[specIndex].integrationTime = (short) atoi(tok);
            }
            else if(strcmp(tok,"SCANS_PER_SAMPLE") == 0){
                tok = strtok(NULL,"\n\d");
                parameters[specIndex].scansPerSample = (short) atoi(tok);
            }
            else if(strcmp(tok,"BOXCAR") == 0){
                tok = strtok(NULL,"\n\d");
                parameters[specIndex].boxcarSmoothing = (short) atoi(tok);
            }
            else if(strcmp(tok,"ABSORBANCE_WAVELENGTHS") == 0){
                wavelengthCount = 0;
                tok = strtok(NULL,",\n\d");
                while(tok != NULL){
                    parameters[specIndex].absorbingWavelengths[wavelengthCount] = atof(tok); 
                    wavelengthCount++;
                    tok = strtok(NULL,",\n\d");
                }
                parameters[specIndex].absorbingWavelengthCount = wavelengthCount; 
            }
           else if(strcmp(tok,"NON_ABSORBING_WAVELENGTH") == 0){
                tok = strtok(NULL,"\n\d");
                parameters[specIndex].nonAbsorbingWavelength = atof(tok);
            }
            else{ // Unrecognized spectrometer parameter, skip this line
                syslog(LOG_DAEMON||LOG_INFO,"Skipped Unrecognized Config Line: %s",configLine);
                continue;
            }
        }
    }
    fclose(configFile);
    return 1;
}

void writeConfigFile(){
    int i,j;
    FILE* configFile;

    if(access(CONFIGPATH,R_OK||W_OK)){
        return; // Do not have access to file
    }

    configFile = fopen(CONFIGPATH,"w+");

    for(i=0; i < 2; i++){
        fprintf(configFile,"# Spectrometer %d Parameters\n",i+1);
        fprintf(configFile,"SPEC%d.SERIAL=%s\n",i+1,parameters[i].serial);
        fprintf(configFile,"SPEC%d.INTEGRATION_TIME=%d\n",i+1,parameters[i].integrationTime);
        fprintf(configFile,"SPEC%d.SCANS_PER_SAMPLE=%d\n",i+1,parameters[i].scansPerSample);
        fprintf(configFile,"SPEC%d.BOXCAR=%d\n",i+1,parameters[i].boxcarSmoothing);
        fprintf(configFile,"\n"); 
        fprintf(configFile,"SPEC%d.ABSORBANCE_WAVELENGTHS=",i+1);
        for(j=0; j < parameters[i].absorbingWavelengthCount; j++){
            if(j==0)
              fprintf(configFile,"%f",parameters[i].absorbingWavelengths[j]);
            else
                fprintf(configFile,",%f",parameters[i].absorbingWavelengths[j]); 
        }
        fprintf(configFile,"\nSPEC%d.NON_ABSORBING_WAVELENGTH=%f\n",i+1,parameters[i].nonAbsorbingWavelength);
        fprintf(configFile,"\n");
    }
    fclose(configFile);

}

char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing){
    parameters[specIndex].integrationTime = newIntTime;
    parameters[specIndex].scansPerSample = newScansPerSample;
    parameters[specIndex].boxcarSmoothing = newBoxcarSmoothing;

    writeConfigFile();
}

char setComputationData(int specIndex, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave){
    int i;

    parameters[specIndex].absorbingWavelengthCount = newAbsWaveCount;
    for(i=0; i < newAbsWaveCount; i++){
        parameters[specIndex].absorbingWavelengths[i] = newAbsWaves[i];
    }
    parameters[specIndex].nonAbsorbingWavelength = newNonAbsWave;    
 
    writeConfigFile();
}

const char* getSerialNumber(int specIndex){
    return parameters[specIndex].serial;
}
const unsigned short getIntegrationTime(int specIndex){
    return parameters[specIndex].integrationTime;
}
const unsigned short getScansPerSample(int specIndex){
    return parameters[specIndex].scansPerSample;
}
const unsigned short getBoxcarSmoothing(int specIndex){
    return parameters[specIndex].boxcarSmoothing;
}
const unsigned char getAbsorbingWavelengthCount(int specIndex){
    return parameters[specIndex].absorbingWavelengthCount;
}
const float* getAbsorbingWavelengths(int specIndex){
    return parameters[specIndex].absorbingWavelengths;
}
const float getNonAbsorbingWavelength(int specIndex){
    return parameters[specIndex].nonAbsorbingWavelength;
}

void logConfig(){
    int i,j;

    for(i=0; i < 2; i++){
        syslog(LOG_DAEMON||LOG_INFO,"Spectrometer %i Configuration",i+1);
        syslog(LOG_DAEMON||LOG_INFO,"=============================");
        syslog(LOG_DAEMON||LOG_INFO,"Serial Number: %s",parameters[i].serial);
        syslog(LOG_DAEMON||LOG_INFO,"Integration Time: %d",parameters[i].integrationTime);
        syslog(LOG_DAEMON||LOG_INFO,"Scans Per Sample: %d",parameters[i].scansPerSample);
        syslog(LOG_DAEMON||LOG_INFO,"Boxcar Smoothing: %d",parameters[i].boxcarSmoothing);
        syslog(LOG_DAEMON||LOG_INFO,"Absorbing Wavelength Count: %d",parameters[i].absorbingWavelengthCount);
        for(j=0; j < parameters[i].absorbingWavelengthCount; j++){
            syslog(LOG_DAEMON||LOG_INFO,"Absorbing Wavelength %i: %f",j+1,parameters[i].absorbingWavelengths[j]);
        }
        syslog(LOG_DAEMON||LOG_INFO,"Non Absorbing Wavelength: %f",parameters[i].nonAbsorbingWavelength); 
    }
}

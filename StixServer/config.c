#include "config.h"
#include "USB4000Manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslog.h>

#define CONFIGPATH "/media/card/StixServerConfig.txt"
#define MAXCONFIGLINE 128

specConfig config[2];

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
                tok = strtok(NULL,"\n");
                strncpy(config[specIndex].serial,tok,11);
            }
            else if(strcmp(tok,"INTEGRATION_TIME") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.integrationTime = (short) atoi(tok);
            }
            else if(strcmp(tok,"SCANS_PER_SAMPLE") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.scansPerSample = (short) atoi(tok);
            }
            else if(strcmp(tok,"BOXCAR") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.boxcarSmoothing = (short) atoi(tok);
            }
            else if(strcmp(tok,"ABSORBANCE_WAVELENGTHS") == 0){
                wavelengthCount = 0;
                tok = strtok(NULL,",\n");
                while(tok != NULL){
                    config[specIndex].waveParameters.absorbingWavelengths[wavelengthCount] = atof(tok); 
                    wavelengthCount++;
                    tok = strtok(NULL,",\n");
                }
                config[specIndex].waveParameters.absorbingWavelengthCount = wavelengthCount; 
            }
           else if(strcmp(tok,"NON_ABSORBING_WAVELENGTH") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.nonAbsorbingWavelength = atof(tok);
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

void applyConfig(){
    int i,j;
  
    for(i=0; i < NUM_SPECS; i++)
    {
        setSpecIntegrationTimeinMilli(i,getIntegrationTime(i)); 
        for(j=0; j < getAbsorbingWavelengthCount(i); j++)
        {
            config[i].absCalcParameters.absorbingPixels[j] = calcPixelValueForWavelength(i,config[i].waveParameters.absorbingWavelengths[j]);
        }
        config[i].absCalcParameters.nonAbsorbingPixel = calcPixelValueForWavelength(i,config[i].waveParameters.nonAbsorbingWavelength);
    }
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
        fprintf(configFile,"SPEC%d.SERIAL=%s\n",i+1,config[i].serial);
        fprintf(configFile,"SPEC%d.INTEGRATION_TIME=%d\n",i+1,config[i].specParameters.integrationTime);
        fprintf(configFile,"SPEC%d.SCANS_PER_SAMPLE=%d\n",i+1,config[i].specParameters.scansPerSample);
        fprintf(configFile,"SPEC%d.BOXCAR=%d\n",i+1,config[i].specParameters.boxcarSmoothing);
        fprintf(configFile,"\n"); 
        fprintf(configFile,"SPEC%d.ABSORBANCE_WAVELENGTHS=",i+1);
        for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
            if(j==0)
              fprintf(configFile,"%f",config[i].waveParameters.absorbingWavelengths[j]);
            else
                fprintf(configFile,",%f",config[i].waveParameters.absorbingWavelengths[j]); 
        }
        fprintf(configFile,"\nSPEC%d.NON_ABSORBING_WAVELENGTH=%f\n",i+1,config[i].waveParameters.nonAbsorbingWavelength);
        fprintf(configFile,"\n");
    }
    fclose(configFile);

}

char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing){
    config[specIndex].specParameters.integrationTime = newIntTime;
    setSpecIntegrationTimeinMilli(specIndex,config[specIndex].specParameters.integrationTime);
    config[specIndex].specParameters.scansPerSample = newScansPerSample;
    config[specIndex].specParameters.boxcarSmoothing = newBoxcarSmoothing;
}

char setComputationData(int specIndex, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave){
    int i;

    config[specIndex].waveParameters.absorbingWavelengthCount = newAbsWaveCount;
    for(i=0; i < newAbsWaveCount; i++){
        config[specIndex].waveParameters.absorbingWavelengths[i] = newAbsWaves[i];
        config[specIndex].absCalcParameters.absorbingPixels[i] = calcPixelValueForWavelength(specIndex,newAbsWaves[i]);
    }
    config[specIndex].waveParameters.nonAbsorbingWavelength = newNonAbsWave;    
    config[specIndex].absCalcParameters.nonAbsorbingPixel = calcPixelValueForWavelength(specIndex,newNonAbsWave);
    applyConfig();
    writeConfigFile();
}

spectrometerParameters* getSpecParameters(int specIndex){
    return &(config[specIndex].specParameters);
}

wavelengthParameters* getWaveParameters(int specIndex){
    return &(config[specIndex].waveParameters);
}

char* getSerialNumber(int specIndex){
    return config[specIndex].serial;
}
unsigned short getIntegrationTime(int specIndex){
    return config[specIndex].specParameters.integrationTime;
}
unsigned short getScansPerSample(int specIndex){
    return config[specIndex].specParameters.scansPerSample;
}
unsigned short getBoxcarSmoothing(int specIndex){
    return config[specIndex].specParameters.boxcarSmoothing;
}
unsigned char getAbsorbingWavelengthCount(int specIndex){
    return config[specIndex].waveParameters.absorbingWavelengthCount;
}
float* getAbsorbingWavelengths(int specIndex){
    return config[specIndex].waveParameters.absorbingWavelengths;
}
float getNonAbsorbingWavelength(int specIndex){
    return config[specIndex].waveParameters.nonAbsorbingWavelength;
}

unsigned short* getAbsorbancePixels(int specIndex){
    return config[specIndex].absCalcParameters.absorbingPixels;
}

unsigned short getNonAbsorbancePixel(int specIndex){
    return config[specIndex].absCalcParameters.nonAbsorbingPixel;
}

void logConfig(){
    int i,j;

    for(i=0; i < 2; i++){
        syslog(LOG_DAEMON||LOG_INFO,"Spectrometer %i Configuration",i+1);
        syslog(LOG_DAEMON||LOG_INFO,"=============================");
        syslog(LOG_DAEMON||LOG_INFO,"Serial Number: %s",config[i].serial);
        syslog(LOG_DAEMON||LOG_INFO,"Integration Time: %d",config[i].specParameters.integrationTime);
        syslog(LOG_DAEMON||LOG_INFO,"Scans Per Sample: %d",config[i].specParameters.scansPerSample);
        syslog(LOG_DAEMON||LOG_INFO,"Boxcar Smoothing: %d",config[i].specParameters.boxcarSmoothing);
        syslog(LOG_DAEMON||LOG_INFO,"Absorbing Wavelength Count: %d",config[i].waveParameters.absorbingWavelengthCount);
        for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
            syslog(LOG_DAEMON||LOG_INFO,"Absorbing Wavelength %i: %f",j+1,config[i].waveParameters.absorbingWavelengths[j]);
        }
        syslog(LOG_DAEMON||LOG_INFO,"Non Absorbing Wavelength: %f",config[i].waveParameters.nonAbsorbingWavelength); 
    }
}

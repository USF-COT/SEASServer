/* 
 * File:   config.h
 * Author: Michael Lindemuth
 *
 * Created on June 3, 2009, 1:00 PM
 */

#ifndef _CONFIG_H
#define	_CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "USB4000Manager.h"
#include "methodFileRunner.h"
#include "dataFileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslog.h>
#include <pthread.h>

// If MAX_ABS_WAVES is changed, check dataFileManager.c insertStmt
#define MAX_ABS_WAVES 9
#define MAX_ANA_NAME 24

/* Units */
enum  CONCENTRATION_UNITS   {

 PICOMOLAR   =  0,
 NANOMOLAR,
 MICROMOLAR,
 MOLAR,
 MAX_CONCENTRATION_UNITS

};

/* Mode */
typedef enum SYS_MODE {
    MANUAL,
    PROGRAM
}systemMode;

typedef struct SPECTROMETERPARAMETERS{
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;
}spectrometerParameters;

typedef struct WAVELENGTHPARAMETERS{
    char analyteName[MAX_ANA_NAME];
    unsigned char units;
    unsigned char absorbingWavelengthCount;
    unsigned char systemMeasureMode;
    unsigned char cMeasureMode;
    float absorbingWavelengths[MAX_ABS_WAVES];
    float nonAbsorbingWavelength;
    float temperature;
    float CtS1;
    float pCO2S1;
    float pCO2S2;
    float slope[MAX_ABS_WAVES];
    float intercept[MAX_ABS_WAVES]; 
}wavelengthParameters;

typedef struct ABSORBANCECALCPARAMETERS{
    unsigned short absorbingPixels[MAX_ABS_WAVES];
    unsigned short nonAbsorbingPixel;
}absorbanceCalcParameters;

typedef struct SPECCONFIG{
    char serial[12];
    uint16_t dwell;
    spectrometerParameters specParameters;
    wavelengthParameters waveParameters; 
    absorbanceCalcParameters absCalcParameters;
}specConfig;

// Config File Managment Methods
void applyConfig();
void writeConfigFile();
char readConfig();

// Set Methods
void setMode(systemMode sysMode);
char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing);
char setComputationData(int specIndex, char* newAnalyteName, unsigned char newUnits, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave);
void setAbsorbanceWavelengths(int specIndex,unsigned char newAbsWaveCount,float* newAbsWaves);
void setNonAbsorbingWavelengths(int specIndex,float newNonAbsWave);
void setDwell(int specIndex, uint16_t dwell);
void setComputationDataBytes(int specIndex,unsigned char* bytes);
void setSlopeIntercept(int specIndex, float* slopeInterceptPairs);

// Get Methods
specConfig* getConfigCopy(int specIndex);
void freeSpecConfig(specConfig* config);
systemMode getMode();
char* getSerialNumber(int specIndex);
spectrometerParameters* getSpecParameters(int specIndex);
wavelengthParameters* getWaveParameters(int specIndex);
unsigned short getIntegrationTime(int specIndex);
unsigned short getScansPerSample(int specIndex);
unsigned short getBoxcarSmoothing(int specIndex);
unsigned char getAbsorbingWavelengthCount(int specIndex);
float* getAbsorbingWavelengths(int specIndex);
float getNonAbsorbingWavelength(int specIndex);
unsigned short* getAbsorbancePixels(int specIndex);
unsigned short getNonAbsorbancePixel(int specIndex);
uint16_t getDwell(int specIndex);
float* getSlopes(int specIndex);
float* getIntercepts(int specIndex);

// Config Debug Methods
void logConfig();

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */


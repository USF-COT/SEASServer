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

#define MAX_ABS_WAVES 9

typedef struct SPECTROMETERPARAMETERS{
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;
}spectrometerParameters;

typedef struct WAVELENGTHPARAMETERS{
    unsigned char absorbingWavelengthCount;
    char reserved[3];
    float absorbingWavelengths[MAX_ABS_WAVES];
    float nonAbsorbingWavelength;
}wavelengthParameters;

typedef struct ABSORBANCECALCPARAMETERS{
    unsigned short absorbingPixels[MAX_ABS_WAVES];
    unsigned short nonAbsorbingPixel;
}absorbanceCalcParameters;

typedef struct SPECCONFIG{
    char serial[12];
    spectrometerParameters specParameters;
    wavelengthParameters waveParameters; 
    absorbanceCalcParameters absCalcParameters;
}specConfig;

void writeConfigFile();
char readConfig();
char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing);
char setComputationData(int specIndex, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave);
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
void logConfig();

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */


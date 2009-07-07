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

typedef struct SPECTROMETERPARAMETERS{
    char serial[12];
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;
    unsigned char absorbingWavelengthCount;
    float absorbingWavelengths[9];
    float nonAbsorbingWavelength;
}spectrometerParameters;

char readConfig();
char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing);
char setComputationData(int specIndex, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave);
char* getSerialNumber(int specIndex);
unsigned short getIntegrationTime(int specIndex);
unsigned short getScansPerSample(int specIndex);
unsigned short getBoxcarSmoothing(int specIndex);
unsigned char getAbsorbingWavelengthCount(int specIndex);
float* getAbsorbingWavelengths(int specIndex);
float getNonAbsorbingWavelength(int specIndex);
void logConfig();

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */


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
const char* getSerialNumber(int specIndex);
const unsigned short getIntegrationTime(int specIndex);
const unsigned short getScansPerSample(int specIndex);
const unsigned short getBoxcarSmoothing(int specIndex);
const unsigned char getAbsorbingWavelengthCount(int specIndex);
const float* getAbsorbingWavelengths(int specIndex);
const float getNonAbsorbingWavelength(int specIndex);
void logConfig();

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */


/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include <stdio.h>
#include <pthread.h>
#include <sys/syslog.h>
#include "USB4000Gum.h"
#include "config.h"
#include "absorbanceCalculation.h"

#ifndef _USB4000MANAGER_H
#define _USB4000MANAGER_H

#define CONNECTED 1
#define DISCONNECTED 0
#define CONNECT_OK 1
#define CONNECT_ERR 0
#define NUM_SPECS 2

int connectSpectrometers(char* serialNumber[]);
void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime);
calibrationCoefficients* getCalCos(char specNumber);
void recordDarkSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds);
void recordRefSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds);
void recordSpecSampe(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds);
specSample* getSpecSample(char specNumber,unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds);
float* getAbsorbance(char specNumber);
unsigned short calcPixelValueForWavelength(unsigned char specNumber,float wavelength);

int disconnectSpectrometers();

#endif

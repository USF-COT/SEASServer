/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include <stdio.h>
#include <pthread.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include "USB4000Gum.h"
#include "config.h"
#include "absorbanceCalculation.h"
#include "GUIprotocol.h"

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
float* getAbsorbance(unsigned char specNumber);
float* getAbsorbanceSpectrum(unsigned char specNumber);
unsigned short calcPixelValueForWavelength(unsigned char specNumber,float wavelength);

void sendSpecSample(int connection, char* command);
void sendCalCos(int connection, char* command);
void sendAbsorbance(int connection, char* command);
void sendAbsorbanceSpectrum(int connection, char* command);
void receiveRecordDarkSample(int connection, char* command);
void receiveRecordRefSample(int connection, char* command);
void receiveRecordSpecSample(int connection, char* command);

int disconnectSpectrometers();

#endif

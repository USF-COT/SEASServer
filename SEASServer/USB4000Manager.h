/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#ifndef _USB4000MANAGER_H
#define _USB4000MANAGER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <inttypes.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include "libUSB4000.h"
#include "config.h"
#include "absorbanceCalculation.h"
#include "GUIprotocol.h"
#include "MethodParser/MethodNodesTable.h"
#include "runProtocol.h"
#include "globalIncludes.h"
#include "SEASPeripheralCommands.h"

#define CONNECTED 1
#define DISCONNECTED 0
#define CONNECT_OK 1
#define CONNECT_ERR 0
#define NUM_SPECS 2

int connectSpectrometers(char* serialNumber[]);
void sleepSpectrometer(short specID);
void wakeSpectrometer(short specID);

unsigned short getNumPixels(char specNumber);
void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime);
calibrationCoefficients* getCalCos(char specNumber);
void recordDarkSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds, unsigned short boxcar);
void recordRefSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds, unsigned short boxcar);
void recordSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds,unsigned short boxcar);

// Get Methods
specSample* getSpecSample(char specNumber,unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds,unsigned short boxcar);
specSample* getDefaultSample(char specNumber);
float* getRawCounts(unsigned char specNumber);
float* getAbsorbance(unsigned char specNumber);
float* getCorrectionAbsorbance(unsigned char origSpecNumber, unsigned char corrSpecNumber);
float* getAbsorbanceSpectrum(unsigned char specNumber);
unsigned short calcPixelValueForWavelength(unsigned char specNumber,float wavelength);
float* getConcentrations(unsigned char specNumber);

// Get last methods
double* getWavelengths(char specNumber);
specSample* getRefSample(char specNumber);
specSample* getLastSample(char specNumber);

void sendSpecSample(int connection, char* command);
void sendCalCos(int connection, char* command);
void sendAbsorbance(int connection, char* command);
void sendAbsorbanceSpectrum(int connection, char* command);
void sendConcentration(int connection, char* command);
void receiveRecordDarkSample(int connection, char* command);
void receiveRecordRefSample(int connection, char* command);
void receiveRecordSpecSample(int connection, char* command);

// Method File Commands
void methodReadReference(unsigned long argc, void* argv);
void methodReadSample(unsigned long argc, void* argv);
void methodReadFullSpec(unsigned long argc, void* argv);
void methodAbsCorr(unsigned long argc, void* argv);
void methodCalcConc(unsigned long argc, void* argv);
void methodCalcPCO2(unsigned long argc, void* argv);
void methodCalcPH(unsigned long argc, void* argv);

// Run Protocol Methods
void readRefRunResponse(int connection, s_node* node);
void readSampRunResponse(int connection, s_node* node);
void calConcRunResponse(int connection, s_node* node);
void calPCO2RunResponse(int connection, s_node* node);
void calPHRunResponse(int connection, s_node* node);
void calTCRunResponse(int connection, s_node* node);
void readFullSpecRunResponse(int connection, s_node* node);

int disconnectSpectrometers();

#ifdef __cplusplus
 }
#endif

#endif

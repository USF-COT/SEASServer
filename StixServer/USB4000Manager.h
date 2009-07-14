/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include <stdio.h>
#include <pthread.h>
#include <sys/syslog.h>
#include "USB4000Gum.h"

#ifndef _USB4000MANAGER_H
#define _USB4000MANAGER_H

#define CONNECTED 1
#define DISCONNECTED 0
#define CONNECT_OK 1
#define CONNECT_ERR 0

int connectSpectrometers(const char* serialNumber1, const char* serialNumber2);
void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime);
calibrationCoefficients* getCalCos(char specNumber);
specSample* getSpecSample(char specNumber,unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds);

int disconnectSpectrometers();

#endif

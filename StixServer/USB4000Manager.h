/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#ifndef _USB4000MANAGER_H
#define _USB4000MANAGER_H

#include <stdio.h>
#include <pthread.h>
#include "USB4000.h"

#define CONNECTED 1
#define DISCONNECTED 0
#define CONNECT_OK 1
#define CONNECT_ERR 0

spectrometer* spectrometers[2];
char connected = DISCONNECTED;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int connect(char* serialNumber1, char* serialNumber2);

calibrationCoefficients* getCalCos(char specNumber);
specSample* getSpecSample(char specNumber);

int disconnect();

#endif

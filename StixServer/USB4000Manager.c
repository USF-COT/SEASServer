/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include "USB4000Manager.h"

int connect(char* serialNumber1, char* serialNumber2){
    if(connected)
        return CONNECT_OK;
    else
    {
        pthread_mutex_lock(&mutex);
        spectrometers[0] = openUSB4000("USB4F02572");
        connected = CONNECTED;
        pthread_mutex_unlock(&mutex);
    }
}

calibrationCoefficients* getCalCos(char specNumber);
specSample* getSpecSample(char specNumber);

int disconnect();

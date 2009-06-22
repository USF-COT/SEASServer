/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include "USB4000Manager.h"

spectrometer* spectrometers[2];
char specsConnected = DISCONNECTED;
pthread_mutex_t specsMutex = PTHREAD_MUTEX_INITIALIZER;

int connectSpectrometers(char* serialNumber1, char* serialNumber2){
    if(specsConnected){
        syslog(LOG_DAEMON||LOG_INFO,"USB4000 Already Connected.");
        return CONNECT_OK;
    }
    else
    {
        pthread_mutex_lock(&specsMutex);
        spectrometers[0] = openUSB4000(serialNumber1);
        specsConnected = CONNECTED;
        pthread_mutex_unlock(&specsMutex);
        return CONNECT_OK;
    }
}

calibrationCoefficients* getCalCos(char specNumber){
    calibrationCoefficients* calCos = malloc(sizeof(calibrationCoefficients));

    pthread_mutex_lock(&specsMutex);
    switch(specNumber){
        case 0:
            memcpy(calCos,spectrometers[0]->calibration,sizeof(calibrationCoefficients));
            break;
        case 1:
            memcpy(calCos,spectrometers[1]->calibration,sizeof(calibrationCoefficients));
            break;
        default:
            syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
            calCos = NULL;
            break;
    }
    pthread_mutex_unlock(&specsMutex);

    return calCos;
}
specSample* getSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds){
    specSample* sample = malloc(sizeof(specSample));

    pthread_mutex_lock(&specsMutex);
    switch(specNumber){
        case 0:
            getSample(spectrometers[0], numScansPerSample, delayBetweenScansInMicroSeconds);
            memcpy(sample,spectrometers[0]->sample,sizeof(specSample));
            break;
        case 1:
            getSample(spectrometers[1], numScansPerSample, delayBetweenScansInMicroSeconds);
            memcpy(sample,spectrometers[1]->sample,sizeof(specSample));
            break;
        default:
            syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
            sample = NULL;
            break;
    }
    pthread_mutex_unlock(&specsMutex);

    return sample;
}

int disconnectSpectrometers(){
    if(!specsConnected)
        return CONNECT_OK;
    else{
        pthread_mutex_lock(&specsMutex);
        closeUSB4000(spectrometers[0]);
        specsConnected = DISCONNECTED;
        pthread_mutex_unlock(&specsMutex);
        return CONNECT_OK;
    }
}

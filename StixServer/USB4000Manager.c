/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include "USB4000Manager.h"

spectrometer* spectrometers[2];
char specsConnected = DISCONNECTED;
pthread_mutex_t specsMutex = PTHREAD_MUTEX_INITIALIZER;

int connectSpectrometers(const char* serialNumber1, const char* serialNumber2){
    if(specsConnected){
        syslog(LOG_DAEMON||LOG_INFO,"USB4000 Already Connected.");
        return CONNECT_OK;
    }
    else
    {
        pthread_mutex_lock(&specsMutex);
        syslog(LOG_DAEMON||LOG_INFO,"Connecting Spectrometer %s",serialNumber1);
        spectrometers[0] = openUSB4000(serialNumber1);
        setSpecIntegrationTimeinMilli(0,getIntegrationTime(0));
        syslog(LOG_DAEMON||LOG_INFO,"Connecting Spectrometer %s",serialNumber2);
        spectrometers[1] = openUSB4000(serialNumber2);
        setSpecIntegrationTimeinMilli(1,getIntegrationTime(1)); 
        specsConnected = CONNECTED;
        pthread_mutex_unlock(&specsMutex);
        return CONNECT_OK;
    }
}

void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime){
    setIntegrationTime(spectrometers[specID],integrationTime*1000);
}

calibrationCoefficients* getCalCos(char specNumber){
    calibrationCoefficients* calCos = NULL;
    calibrationCoefficients* original = NULL;

    pthread_mutex_lock(&specsMutex);
    if(specNumber == 0 || specNumber == 1)
    {
        calCos = malloc(sizeof(calibrationCoefficients));  
        original = spectrometers[specNumber]->calibration;
        memcpy(calCos,original,sizeof(calibrationCoefficients));
    }
    else
    {
        syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        calCos = NULL;
    }
    pthread_mutex_unlock(&specsMutex);

    return calCos;
}
specSample* getSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds){
    specSample* sample = NULL;
    specSample* original = NULL;

    pthread_mutex_lock(&specsMutex);

    if(specNumber == 0 || specNumber == 1)
    {
        original = getSample(spectrometers[specNumber], numScansPerSample, delayBetweenScansInMicroSeconds);
        sample = malloc(sizeof(specSample));
        sample->numScansForSample = original->numScansForSample;
        sample->pixels = malloc(sizeof(float)*3840);
        memcpy(sample->pixels,original->pixels,sizeof(float)*3840);
    }
    else
    {
        syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        sample = NULL;
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
        closeUSB4000(spectrometers[1]);
        specsConnected = DISCONNECTED;
        pthread_mutex_unlock(&specsMutex);
        return CONNECT_OK;
    }
}

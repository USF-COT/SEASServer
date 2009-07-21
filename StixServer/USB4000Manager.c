/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include "USB4000Manager.h"

spectrometer* spectrometers[NUM_SPECS];
char specsConnected = DISCONNECTED;
pthread_mutex_t specsMutex[NUM_SPECS];

int connectSpectrometers(char* serialNumber[]){
    int i;

    if(specsConnected)
    {
        syslog(LOG_DAEMON||LOG_INFO,"USB4000 Already Connected.");
        return CONNECT_OK;
    }
    else
    {
        for(i=0; i < NUM_SPECS; i++)
        {
            pthread_mutex_init(&specsMutex[i],NULL);
	    pthread_mutex_lock(&specsMutex[i]);
	    syslog(LOG_DAEMON||LOG_INFO,"Connecting Spectrometer %s",serialNumber[i]);
	    spectrometers[i] = openUSB4000(serialNumber[i]);
	    setSpecIntegrationTimeinMilli(i,getIntegrationTime(i));
            pthread_mutex_unlock(&specsMutex[i]);
        }
        specsConnected = CONNECTED;
        return CONNECT_OK;
    }
}

void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime){
    setIntegrationTime(spectrometers[specID],integrationTime*1000);
}

calibrationCoefficients* getCalCos(char specNumber){
    int i;
    calibrationCoefficients* calCos = NULL;
    calibrationCoefficients* original = NULL;

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        calCos = malloc(sizeof(calibrationCoefficients));  
        original = spectrometers[specNumber]->calibration;
        memcpy(calCos,original,sizeof(calibrationCoefficients));
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        calCos = NULL;
    }

    return calCos;
}

void recordDarkSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readDarkSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds);
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

void recordRefSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readRefSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds);
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

specSample* getSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds){
    specSample* sample = NULL;
    specSample* original = NULL;

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        original = getSample(spectrometers[specNumber], numScansPerSample, delayBetweenScansInMicroSeconds);
        sample = malloc(sizeof(specSample));
        sample->numScansForSample = original->numScansForSample;
        sample->pixels = malloc(sizeof(float)*3840);
        memcpy(sample->pixels,original->pixels,sizeof(float)*3840);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        sample = NULL;
    }

    return sample;
}

unsigned short calcPixelValueForWavelength(unsigned char specNumber,float wavelength)
{
    unsigned short pixel = 0;
    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[i]);
        pixel = GetPixelForWavelength(spectrometers[i],wavelength);
        pthread_mutex_unlock(&specsMutex[i]);
    }
    return pixel;
}       

int disconnectSpectrometers(){
    int i;
    if(!specsConnected)
        return CONNECT_OK;
    else{
        for(i=0; i < NUM_SPECS; i++)
        {
            pthread_mutex_lock(&specsMutex[i]);
            closeUSB4000(spectrometers[i]);
            pthread_mutex_unlock(&specsMutex[i]);
            pthread_mutex_destroy(&specsMutex[i]);
        }
        specsConnected = DISCONNECTED;
        return CONNECT_OK;
    }
}

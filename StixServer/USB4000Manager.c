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
            if(spectrometers[i] == NULL)
                return CONNECT_ERR;
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

void sendCalCos(int connection, char* command){
    syslog(LOG_DAEMON||LOG_INFO,"Retrieving calibration coefficients from USB4000");
    calibrationCoefficients* calCos;

    calCos = getCalCos(command[1]);
    if(calCos){
        send(connection,(void*) calCos,sizeof(calibrationCoefficients),0);
        free(calCos);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Retrieved calibration coefficients. Returning.");
}

void recordDarkSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readDarkSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds);
        if(spectrometers[specNumber]->darkSample == NULL)
            syslog(LOG_DAEMON||LOG_INFO,"Dark sample NULL after attempted read.");
        else
            syslog(LOG_DAEMON||LOG_INFO,"Dark sample read OK.");
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

void recordRefSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readRefSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds);
        if(spectrometers[specNumber]->refSample == NULL)
            syslog(LOG_DAEMON||LOG_INFO,"Reference sample NULL after attempted read.");
        else
            syslog(LOG_DAEMON||LOG_INFO,"Reference sample read OK.");
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

void recordSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds){
    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        getSample(spectrometers[specNumber], numScansPerSample, delayBetweenInMicroSeconds);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON||LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
    }
}

specSample* getSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds){
    specSample* sample = NULL;
    specSample* original = NULL;

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        original = getSample(spectrometers[specNumber], numScansPerSample, delayBetweenScansInMicroSeconds);
        sample = copySample(original,spectrometers[specNumber]->status->numPixels);
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
        pthread_mutex_lock(&specsMutex[specNumber]);
        pixel = GetPixelForWavelength(spectrometers[specNumber],wavelength);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    return pixel;
}       

float* getAbsorbance(unsigned char specNumber)
{
    unsigned char i;
    float* absorbanceValues = calloc(MAX_ABS_WAVES+1,sizeof(float));
    unsigned short* absPixels = getAbsorbancePixels(specNumber);
    unsigned short nonAbsPixel = getNonAbsorbancePixel(specNumber);    

    if(specNumber < NUM_SPECS)
    {
        unsigned char i;
        pthread_mutex_lock(&specsMutex[specNumber]);
        for(i=0; i < getAbsorbingWavelengthCount(specNumber);i++)
        {
            absorbanceValues[i] = ComputeAbsorbance(spectrometers[specNumber],absPixels[i],nonAbsPixel);
        }
        absorbanceValues[MAX_ABS_WAVES] = ComputeCorrectionAbsorbance(spectrometers[specNumber],nonAbsPixel);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }

    return absorbanceValues;
}

float* getAbsorbanceSpectrum(unsigned char specNumber){
    int i;
    float* absValues = NULL;
    unsigned short nonAbsPixel = getNonAbsorbancePixel(specNumber);

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        absValues = malloc(sizeof(float) * spectrometers[specNumber]->status->numPixels);
        getSample(spectrometers[specNumber],getScansPerSample(specNumber),100);
        for(i=0; i < spectrometers[specNumber]->status->numPixels; i++)
        {
            absValues[i] = ComputeAbsorbance(spectrometers[specNumber],i,nonAbsPixel);
        }
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    return absValues;
             
}

void sendSpecSample(int connection, char* command){
    specSample* sample;

    syslog(LOG_DAEMON||LOG_INFO,"Sending wavelength sample from USB4000");
    sample = getSpecSample(command[1],getScansPerSample(command[1]),100);
    send(connection,(void*)sample->pixels,sizeof(float)*3840,0);
    free(sample);
    syslog(LOG_DAEMON||LOG_INFO,"Sent wavelength.");
}

void sendAbsorbance(int connection, char* command){
    float* absorbance = NULL;

    syslog(LOG_DAEMON||LOG_INFO,"Retrieving Absorbance for Specified Wavelengths.");
    absorbance = getAbsorbance(command[1]);
    if(absorbance){
        send(connection,(void*)absorbance,sizeof(float)*(MAX_ABS_WAVES+1),0);
        free(absorbance);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Absorbance for Specified Wavelengths Retrieved.");
}

void sendAbsorbanceSpectrum(int connection, char* command){
    float* absSpec = NULL;

    syslog(LOG_DAEMON||LOG_INFO,"Retrieving Absorbance Spectrum...");
    absSpec = getAbsorbanceSpectrum(command[1]);
    if(absSpec){
        send(connection,(void*)absSpec,sizeof(float)*3840,0);
        free(absSpec);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Absorbance Spectrum Retrieved.");
}

void receiveRecordDarkSample(int connection, char* command){
    char response[1] = {RDS};

    syslog(LOG_DAEMON||LOG_INFO,"Recording Dark Sample...");
    recordDarkSample(command[1],getScansPerSample(command[1]),100);
    send(connection,(void*)response,1,0);    
    syslog(LOG_DAEMON||LOG_INFO,"Dark Sample Recorded.");
}

void receiveRecordRefSample(int connection, char* command){
    char response[1] = {RRS};
    
    syslog(LOG_DAEMON||LOG_INFO,"Recording Reference Sample...");
    recordRefSample(command[1],getScansPerSample(command[1]),100);
    send(connection,(void*)response,1,0);
    syslog(LOG_DAEMON||LOG_INFO,"Reference Sample Recorded.");
}

void receiveRecordSpecSample(int connection, char* command){
    syslog(LOG_DAEMON||LOG_INFO,"Recording Sample.");
    recordSpecSample(command[1],getScansPerSample(command[1]),100);
    syslog(LOG_DAEMON||LOG_INFO,"Sample Recorded");
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
            pthread_mutex_destroy(&specsMutex[i]);
        }
        specsConnected = DISCONNECTED;
        return CONNECT_OK;
    }
}

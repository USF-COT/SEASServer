/* USB4000Manager - Manages the photospectrometers for connected Stix Server threads.
 * All variables and calls are made thread safe with blocking mutexes on critical sections.
 *
 * By: Michael Lindemuth
 */

#include "USB4000Manager.h"
#include "carbonCalc.h"
#include "dataFileManager.h"

spectrometer* spectrometers[NUM_SPECS];
char specsConnected = DISCONNECTED;
pthread_mutex_t specsMutex[NUM_SPECS];

int connectSpectrometers(char* serialNumber[]){
    int i;

    if(specsConnected)
    {
        syslog(LOG_DAEMON|LOG_INFO,"USB4000 Already Connected.");
        return CONNECT_OK;
    }
    else
    {
        for(i=0; i < NUM_SPECS; i++)
        {
            pthread_mutex_init(&specsMutex[i],NULL);
            pthread_mutex_lock(&specsMutex[i]);
            syslog(LOG_DAEMON|LOG_INFO,"Connecting Spectrometer %s",serialNumber[i]);
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

void sleepSpectrometer(short specID){
    setShutdownMode(spectrometers[specID],FALSE);
}
void wakeSpectrometer(short specID){
    setShutdownMode(spectrometers[specID],TRUE);
}

void setSpecIntegrationTimeinMilli(short specID, unsigned int integrationTime){
    setIntegrationTime(spectrometers[specID],integrationTime*1000);
}

unsigned short getNumPixels(char specNumber){
    unsigned short numPixels = 0;

    if(specNumber < NUM_SPECS){
        pthread_mutex_lock(&specsMutex[specNumber]);
        numPixels = spectrometers[specNumber]->status->numPixels;
        pthread_mutex_unlock(&specsMutex[specNumber]);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Invalid spectrometer index (%d) passed to getNumPixels.",specNumber);
    }

    return numPixels;
}

calibrationCoefficients* getCalCos(char specNumber){
    int i;
    calibrationCoefficients* calCos = NULL;
    calibrationCoefficients* original = NULL;

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        calCos = (calibrationCoefficients*)malloc(sizeof(calibrationCoefficients));  
        original = spectrometers[specNumber]->calibration;
        memcpy(calCos,original,sizeof(calibrationCoefficients));
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        calCos = NULL;
    }

    return calCos;
}

void sendCalCos(int connection, char* command){
    syslog(LOG_DAEMON|LOG_INFO,"Retrieving calibration coefficients from USB4000");
    calibrationCoefficients* calCos;

    calCos = getCalCos(command[1]);
    if(calCos){
        send(connection,(void*)calCos,sizeof(calibrationCoefficients),0);
        free(calCos);
    }
    syslog(LOG_DAEMON|LOG_INFO,"Retrieved calibration coefficients. Returning.");
}

// Utility methods
specSample* getRefSample(char specNumber){
    specSample* refSample = NULL;

    syslog(LOG_DAEMON|LOG_INFO,"Reading ref sample for spectrometer %d. Number of pixels = %d",specNumber,spectrometers[specNumber]->status->numPixels);

    if(specNumber < NUM_SPECS){
        pthread_mutex_lock(&specsMutex[specNumber]);
        if(spectrometers[specNumber]->refSample){
            refSample = copySample(spectrometers[specNumber]->refSample,spectrometers[specNumber]->status->numPixels);
        }
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }

    return refSample;
}

specSample* getLastSample(char specNumber){
    specSample* lastSample = NULL;

    if(specNumber < NUM_SPECS){
        pthread_mutex_lock(&specsMutex[specNumber]);
        if(spectrometers[specNumber]->sample){
            lastSample = copySample(spectrometers[specNumber]->sample,spectrometers[specNumber]->status->numPixels);
        }
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }

    return lastSample;
}

void moveSpecSampleToFloats(float* dest, specSample* sample,int numPixels){
    int i;
    if(sample){
        for(i=0; i < numPixels; i++){
            dest[i] = ((float*)sample->pixels)[i];
        }
        deallocateSample(&sample);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Cannot move sample to floats, sample is NULL.");
    }
}

void recordDarkSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds, unsigned short boxcar){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readDarkSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds,boxcar);
        if(spectrometers[specNumber]->darkSample == NULL)
            syslog(LOG_DAEMON|LOG_INFO,"Dark sample NULL after attempted read.");
        else
            syslog(LOG_DAEMON|LOG_INFO,"Dark sample read OK.");
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

void recordRefSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds,unsigned short boxcar){
    pthread_mutex_lock(&specsMutex[specNumber]);
    if(specNumber == 0 || specNumber == 1)
    {
        readRefSpectra(spectrometers[specNumber],numScansPerSample,delayBetweenInMicroSeconds,boxcar);
        if(spectrometers[specNumber]->refSample == NULL)
            syslog(LOG_DAEMON|LOG_INFO,"Reference sample NULL after attempted read.");
        else
            syslog(LOG_DAEMON|LOG_INFO,"Reference sample read OK.");
    }
    pthread_mutex_unlock(&specsMutex[specNumber]);
}

void recordSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenInMicroSeconds, unsigned short boxcar){
    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        getSample(spectrometers[specNumber], numScansPerSample, delayBetweenInMicroSeconds,boxcar);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index out of range in recordSpecSample method.  Requested spectrometer number %i.",specNumber);
    }
}

specSample* getSpecSample(char specNumber, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds, unsigned short boxcar){
    specSample* sample = NULL;
    specSample* original = NULL;

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        original = getSample(spectrometers[specNumber], numScansPerSample, delayBetweenScansInMicroSeconds,boxcar);
        sample = copySample(original,spectrometers[specNumber]->status->numPixels);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    else
    {
        syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index out of range.  Requested spectrometer number %i.",specNumber);
        sample = NULL;
    }

    return sample;
}

specSample* getDefaultSample(char specNumber){
    return getSpecSample(specNumber,getScansPerSample(specNumber),100,getBoxcarSmoothing(specNumber));
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

float* getRawCounts(unsigned char specNumber){
    uint8_t i;
    float* countValues = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
    unsigned short* absPixels = getAbsorbancePixels(specNumber);
    unsigned short nonAbsPixel = getNonAbsorbancePixel(specNumber);
    specSample* sample = getLastSample(specNumber); 

    if(!countValues){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to allocate memory for counts array.");         
    }

    if(specNumber < NUM_SPECS && absPixels && sample){
        for(i=0; i < getAbsorbingWavelengthCount(specNumber);i++){
            countValues[i] = sample->pixels[absPixels[i]];
        }
        countValues[MAX_ABS_WAVES] = sample->pixels[nonAbsPixel];

        // housekeeping!
        deallocateSample(&sample);
    }
    return countValues;
}

float* getAbsorbance(unsigned char specNumber)
{
    unsigned char i;
    float* absorbanceValues = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
    unsigned short* absPixels = getAbsorbancePixels(specNumber);
    unsigned short nonAbsPixel = getNonAbsorbancePixel(specNumber);    

    if(!absorbanceValues){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to allocate memory for absorbance array.");
        return NULL;
    }

    if(specNumber < NUM_SPECS)
    {
        pthread_mutex_lock(&specsMutex[specNumber]);
        for(i=0; i < getAbsorbingWavelengthCount(specNumber);i++)
        {
            absorbanceValues[i] = ComputeAbsorbance(spectrometers[specNumber],absPixels[i],nonAbsPixel,FALSE,TRUE);
        }
        absorbanceValues[MAX_ABS_WAVES] = ComputeCorrectionAbsorbance(spectrometers[specNumber],nonAbsPixel,FALSE);
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }

    return absorbanceValues;
}

float* getCorrectionAbsorbance(unsigned char origSpecNumber, unsigned char corrSpecNumber){
    unsigned char i;
    float* origAbs = NULL;
    float* corrAbs = NULL;
    float* absorbanceValues = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));

    if(!absorbanceValues){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to allocate memory for correction absorbance array.");
        return NULL;
    }

    if(origSpecNumber < NUM_SPECS && corrSpecNumber < NUM_SPECS){
        origAbs = getAbsorbance(origSpecNumber);
        corrAbs = getAbsorbance(corrSpecNumber);
        if(origAbs && corrAbs){
            for(i=0; i < getAbsorbingWavelengthCount(origSpecNumber); i++){
                absorbanceValues[i] = origAbs[i] - corrAbs[i];
            }
            absorbanceValues[MAX_ABS_WAVES] = (origAbs[MAX_ABS_WAVES] + corrAbs[MAX_ABS_WAVES])/2;
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Error allocating absorbance arrays in getCorrectionAbsorbance method.");
        }
        if(origAbs) free(origAbs);
        if(corrAbs) free(corrAbs);  
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index (%d,%d) is out of bounds.",origSpecNumber,corrSpecNumber);
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
        absValues = (float*)malloc(sizeof(float) * spectrometers[specNumber]->status->numPixels);
        for(i=0; i < spectrometers[specNumber]->status->numPixels; i++)
        {
            absValues[i] = ComputeAbsorbance(spectrometers[specNumber],i,nonAbsPixel,FALSE,FALSE);
        }
        pthread_mutex_unlock(&specsMutex[specNumber]);
    }
    return absValues;

}

float* getConcentrations(unsigned char specNumber){
    int i;

    float* concentrations = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
    if(!concentrations){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to calloc concentration memory.  PROBABLY OUT OF MEMORY!");
        return NULL;
    }

    float* absorbances = getAbsorbance(specNumber);
    if(!absorbances){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve absorbance for spec %d",specNumber);
        free(concentrations);
        return NULL;
    }

    float* slopes = getSlopes(specNumber);
    if(!slopes){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve slopes for spec %d",specNumber);
        free(concentrations);
        free(absorbances);
        return NULL;
    }

    float* intercepts = getIntercepts(specNumber);
    if(!intercepts){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve intercepts for spec %d",specNumber);
        free(concentrations);
        free(absorbances);
        free(slopes);
        return NULL;
    }

    for(i=0; i < getAbsorbingWavelengthCount(specNumber); i++){
        concentrations[i] = (absorbances[i]-intercepts[i])/slopes[i];
    }
    free(absorbances);
    free(slopes);
    free(intercepts);
    return concentrations;
}

void sendSpecSample(int connection, char* command){
    unsigned int scans = 0;
    unsigned short boxcar = 0;
    specSample* sample;

    scans = getScansPerSample(command[1]);
    boxcar = getBoxcarSmoothing(command[1]);
    syslog(LOG_DAEMON|LOG_INFO,"Sending wavelength sample from USB4000[%d] over %d samples",command[1],scans);
    sample = getSpecSample(command[1],scans,100,boxcar);
    if(sample){
        send(connection,(void*)(sample->pixels),sizeof(float)*3840,0);
        deallocateSample(&sample);
        syslog(LOG_DAEMON|LOG_INFO,"Sample sent.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve sample from spectrometer %d for %d scans.",command[1],scans);
    }
}

void sendAbsorbance(int connection, char* command){
    float* absorbance = NULL;

    syslog(LOG_DAEMON|LOG_INFO,"Retrieving Spec %d Absorbance for %d Wavelengths.",command[1],getAbsorbingWavelengthCount(command[1]));
    absorbance = getAbsorbance(command[1]);
    if(absorbance){
        send(connection,(void*)absorbance,sizeof(float)*(MAX_ABS_WAVES+1),0);
        free(absorbance);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read absorbance wavelengths.");
        return;
    }
    syslog(LOG_DAEMON|LOG_INFO,"Absorbance for Specified Wavelengths Retrieved.");
}

void sendAbsorbanceSpectrum(int connection, char* command){
    float* absSpec = NULL;

    syslog(LOG_DAEMON|LOG_INFO,"Retrieving Absorbance Spectrum...");
    absSpec = getAbsorbanceSpectrum(command[1]);
    if(absSpec){
        send(connection,(void*)absSpec,sizeof(float)*3840,0);
        free(absSpec);
    }
    syslog(LOG_DAEMON|LOG_INFO,"Absorbance Spectrum Retrieved.");
}

void sendConcentration(int connection, char* command){
    float* conc = NULL;

    if(command[0] == RCC){
        syslog(LOG_DAEMON|LOG_INFO,"Retrieving Spec %d Concentration for %d Wavelengths.",command[1],getAbsorbingWavelengthCount(command[1]));
        conc = getConcentrations(command[1]);
        if(conc){
            send(connection,(void*)conc,sizeof(float)*MAX_ABS_WAVES,0);
            free(conc);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to read concentrations.");
            return;
        }
        syslog(LOG_DAEMON|LOG_INFO,"Successfully Retrieved Spec %d Concentrations.",command[1]);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect Command Passed to Send Concentration Beginning with %02X.",command[0]);
    }
}

void receiveRecordDarkSample(int connection, char* command){
    char response[1] = {RDS};

    syslog(LOG_DAEMON|LOG_INFO,"Recording Dark Sample...");
    recordDarkSample(command[1],getScansPerSample(command[1]),100,getBoxcarSmoothing(command[1]));
    send(connection,(void*)response,1,0);    
    syslog(LOG_DAEMON|LOG_INFO,"Dark Sample Recorded.");
}

void receiveRecordRefSample(int connection, char* command){
    char response[1] = {RRS};

    syslog(LOG_DAEMON|LOG_INFO,"Recording Reference Sample...");
    recordRefSample(command[1],getScansPerSample(command[1]),100,getBoxcarSmoothing(command[1]));
    send(connection,(void*)response,1,0);
    syslog(LOG_DAEMON|LOG_INFO,"Reference Sample Recorded.");
}

void receiveRecordSpecSample(int connection, char* command){
    syslog(LOG_DAEMON|LOG_INFO,"Recording Sample.");
    recordSpecSample(command[1],getScansPerSample(command[1]),100,getBoxcarSmoothing(command[1]));
    syslog(LOG_DAEMON|LOG_INFO,"Sample Recorded");
}

// Run Protocol Methods
void readRefRunResponse(int connection, s_node* node){
    unsigned short numPixels = 0; // for readability
    unsigned char header[3] = {RTH,READ_REFERENCE_RUNTIME_CMD,0};
    specSample* refSample = NULL;
    READ_REFERENCE_RUNTIME_DATA data;

    memset(&data,0,sizeof(READ_REFERENCE_RUNTIME_DATA));

    if(node->commandID == READ_REFERENCE_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (unsigned char) (((double*)node->argv)[0]-1);

        header[2] = data.Spectrometer;
        // Store Reference Sample
        if(data.Spectrometer < NUM_SPECS){
            numPixels = spectrometers[data.Spectrometer]->status->numPixels;
            refSample = getRefSample(data.Spectrometer);
            if(refSample != NULL){
                moveSpecSampleToFloats(data.Counts,refSample,numPixels); 

                // Send the Structure
                send(connection,(void*)&data,sizeof(READ_REFERENCE_RUNTIME_DATA),0);
                //send(connection,(void*)header,sizeof(unsigned char)*3,0);
                //send(connection,(void*)refSample->pixels,sizeof(float)*numPixels,0);
                //deallocateSample(&refSample); UNNECESSARY it is already free'd in moveSpecSampleToFloats
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to read reference spectrum.  Have you initialized it?");
            }
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index (%d) out of bounds.",data.Spectrometer);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to readRefRunResponse.",node->commandID);
    }
}

void readSampRunResponse(int connection, s_node* node){
    int i=0;
    unsigned short numPixels = 0;
    specSample* sample = NULL;
    float* abs = NULL;
    float* absSpec = NULL;
    READ_SAMPLE_RUNTIME_DATA data;

    if(node->commandID == READ_SAMPLE_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (unsigned char) (((double*)node->argv)[0]-1);
        if(data.Spectrometer < NUM_SPECS){
            numPixels = spectrometers[data.Spectrometer]->status->numPixels;

            // Request all necessary arrays
            sample = getLastSample(data.Spectrometer);
            if(!sample) syslog(LOG_DAEMON|LOG_ERR,"Unable to read last sample from spectrometer.");
            abs = getAbsorbance(data.Spectrometer);
            absSpec = getAbsorbanceSpectrum(data.Spectrometer);

            // Send Sample, If All Arrays Retrieved
            if(sample && abs && absSpec){
                moveSpecSampleToFloats(data.Counts,sample,numPixels);
                for(i=0;i<MAX_ABS_WAVES-1;i++) data.Absorbance[i]=abs[i];
                data.CorrectionAbsorbance = abs[MAX_ABS_WAVES];
                for(i=0;i<numPixels;i++) data.AbsorbanceSpectra[i]=absSpec[i];
                send(connection,(void*)&data,sizeof(READ_SAMPLE_RUNTIME_DATA),0);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve data for read sample runtime response.");
            }

            if(abs){
                free(abs);
            } else {              
                syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve absorbance values.");
            }
            if(absSpec){
                free(absSpec);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve absorbance spectrum.");
            }
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Spectrometer index (%d) out of bounds.",data.Spectrometer);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to readSampRunResponse.",node->commandID);
    }
    syslog(LOG_DAEMON|LOG_INFO,"Read Sample Run Response Method Return Complete.");
}

// Method File Commands
void methodReadReference(unsigned long argc, void* argv){
    double* args = (double*)argv;
    char specNumber = (char)args[0]-1;

    recordRefSample(specNumber,getScansPerSample(specNumber),100,getBoxcarSmoothing(specNumber));

    // TODO: Write Reference to Data File?
}

void methodReadSample(unsigned long argc, void* argv){
    double* args = (double*)argv;
    char specNumber = (char)args[0]-1;
    float* absorbance = NULL;

    recordSpecSample(specNumber, getScansPerSample(specNumber), 100,getBoxcarSmoothing(specNumber));
    absorbance = getAbsorbance(specNumber);

    if(absorbance){
        // TODO: Write Sample to Data File

        free(absorbance);
    }
}

void methodReadFullSpec(unsigned long argc, void* argv){
    double* args = (double*)argv;
    char specNumber = (char)args[0]-1;

    syslog(LOG_DAEMON|LOG_INFO,"METHOD: Recording Full Spectrum.");
    recordSpecSample(specNumber,getScansPerSample(specNumber),100,getBoxcarSmoothing(specNumber));


    // TODO: Write Sample to Data File
}

void methodAbsCorr(unsigned long argc, void* argv){
    double* args = (double*)argv;
    unsigned char origSpecNumber = (unsigned char) args[0]-1;
    unsigned char corrSpecNumber = (unsigned char) args[1]-1;
    float* corrAbs = NULL;

    corrAbs = getCorrectionAbsorbance(origSpecNumber,corrSpecNumber);

    // TODO: Write Correction Absorbance to File
}

void methodCalcConc(unsigned long argc, void* argv){

    if(argc == 1){

    }        
}

void methodCalcPCO2(unsigned long argc, void* argv){

}

void methodCalcPH(unsigned long argc, void* argv){
    unsigned char specIndex, absWaveCount;
    float* abs;
    CTDreadings_s* ctd;
    float pH = -1;

    syslog(LOG_DAEMON|LOG_INFO,"Called method calculate pH");
    if(argc >= 1){
        double* args = (double*)argv;
        specIndex = (unsigned char)args[0]-1; // zero index conversion
        if(specIndex < NUM_SPECS){
            absWaveCount = getAbsorbingWavelengthCount(specIndex);
            abs = getAbsorbance(specIndex);
            ctd = getCTDValues();
            if(ctd && abs){
                if(getpHMeasureMode(specIndex) == MCP){
                    syslog(LOG_DAEMON|LOG_INFO,"Calculating pH using MCP for spec %d",specIndex); 
                    pH = computeSystempHMCP(absWaveCount,abs,abs[MAX_ABS_WAVES],*ctd);
                } else if (getpHMeasureMode(specIndex) == TB){
                    syslog(LOG_DAEMON|LOG_INFO,"Calculating pH using TB for spec %d",specIndex);
                    pH = computeSystempHTB(absWaveCount,abs,abs[MAX_ABS_WAVES],*ctd);
                }

                if(pH >= 0){
                    syslog(LOG_DAEMON|LOG_INFO,"Writing pH");
                    writepHToDB(specIndex,pH,absWaveCount,abs,*ctd);
                } else {
                    syslog(LOG_DAEMON|LOG_ERR,"Could not calculate pH correctly.  Function return -1.");
                }
                free(abs);
                free(ctd);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve CTD reading for pH calculation.");
            }        
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Not enough arguments supplied to methodCalcPH");
    }
}

// Run Response Methods
void calConcRunResponse(int connection, s_node* node){
    CAL_CONCENTRATION_RUNTIME_DATA data;

    if(node->commandID == CAL_CONCENTRATION_RUNTIME_CMD){
    data.Header.HeadByte = RTH;
    data.Header.Command = node->commandID;
    data.Spectrometer = (float)(((double*)node->argv)[0]-1);

    // TODO: Fill in concentration code
    data.Concentration[0] = 0;
    data.RRatio = 0;

    send(connection,(void*)&data,sizeof(CAL_CONCENTRATION_RUNTIME_DATA),0);
    } else {
    syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to calConcRunResponse.",node->commandID);
    }
}

void calPCO2RunResponse(int connection, s_node* node){
    CAL_CONCENTRATION_RUNTIME_DATA data;

    if(node->commandID == CAL_PCO2_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (float)(((double*)node->argv)[0]-1);

        // TODO: Fill in PCO2 code
        data.Concentration[0] = 0;
        data.RRatio = 0;

        send(connection,(void*)&data,sizeof(CAL_CONCENTRATION_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to calPCO2RunResponse.",node->commandID);
    }
}
void calPHRunResponse(int connection, s_node* node){
    CAL_CONCENTRATION_RUNTIME_DATA data;

    if(node->commandID == CAL_PH_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (float)(((double*)node->argv)[0]-1);

        // TODO: Fill in PH code
        data.Concentration[0] = 0;
        data.RRatio = 0;

        send(connection,(void*)&data,sizeof(CAL_CONCENTRATION_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to calPHRunResponse.",node->commandID);
    }
} 

void calTCRunResponse(int connection, s_node* node){
    CAL_CONCENTRATION_RUNTIME_DATA data;

    if(node->commandID == CAL_TC_RUNTIME_CMD){
        data.Header.HeadByte = RTH;
        data.Header.Command = node->commandID;
        data.Spectrometer = (float)(((double*)node->argv)[0]-1);

        // TODO: Fill in concentration code
        data.Concentration[0] = 0;
        data.RRatio = 0;

        send(connection,(void*)&data,sizeof(CAL_CONCENTRATION_RUNTIME_DATA),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to calTCRunResponse.",node->commandID);
    }
}

void readFullSpecRunResponse(int connection, s_node* node){
    RUNTIME_RESPONSE_HEADER response;

    if(node->commandID == READ_FULL_SPECTRUM_RUNTIME_CMD){
        response.HeadByte = RTH;
        response.Command = node->commandID;
        send(connection,(void*)&response,sizeof(RUNTIME_RESPONSE_HEADER),0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Incorrect command ID (%02x) passed to readFullSpecRunResponse.",node->commandID);
    }
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

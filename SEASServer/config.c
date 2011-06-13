#include "config.h"

#define CONFIGPATH "/etc/SEASServer/config.txt"
#define MAXCONFIGLINE 128

static pthread_mutex_t writeConfigMutex = PTHREAD_MUTEX_INITIALIZER;

static systemMode mode = MANUAL;
static specConfig config[NUM_SPECS];

// Config File Management Methods
char readConfig(){

    FILE* configFile;
    char configLine[MAXCONFIGLINE];
    char* tok;
    int specIndex,wavelengthCount;
    int i;

    if(access(CONFIGPATH,R_OK|W_OK)){
        return 0; // Do not have access to file
    }

    configFile = fopen(CONFIGPATH,"r+");
    while(!feof(configFile)){
        fgets(configLine,MAXCONFIGLINE,configFile);
 
        // Skip Comments and Empty Lines
        if(configLine[0] == '#' | strlen(configLine) == 0){
            syslog(LOG_DAEMON|LOG_INFO,"Skipped Commented or Empty Config Line: %s",configLine);
            continue;
        }
        else{
            tok = strtok(configLine,".");
            if(strcmp(tok,"SYS") == 0){
                specIndex = -1;
            }
            else if(strcmp(tok,"SPEC1") == 0){
                specIndex = 0;
            }
            else if(strcmp(tok,"SPEC2") == 0){
                specIndex = 1;
            }
            else{ // Unrecognized prefix, skip this line.
                syslog(LOG_DAEMON|LOG_INFO,"Skipped Unrecognized Prefixed Config Line: %s",configLine);
                continue;
            }
            
            tok = strtok(NULL,"="); 
            if(strcmp(tok,"MODE") == 0){
                tok = strtok(NULL,"\n");
                if(strcmp(tok,"MANUAL") == 0){
                    mode = MANUAL; 
                } else if(strcmp(tok,"PROGRAM") == 0){
                    mode = PROGRAM;
                } else {
                    syslog(LOG_DAEMON|LOG_INFO,"Unrecognized Mode Passed: %s",tok);
                }
                continue;
            }
            else if(strcmp(tok,"SERIAL") == 0){
                tok = strtok(NULL,"\n");
                strncpy(config[specIndex].serial,tok,11);
            }
            else if(strcmp(tok,"DWELL") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].dwell = atoi(tok);
            }
            else if(strcmp(tok,"INTEGRATION_TIME") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.integrationTime = (short) atoi(tok);
            }
            else if(strcmp(tok,"SCANS_PER_SAMPLE") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.scansPerSample = (short) atoi(tok);
            }
            else if(strcmp(tok,"BOXCAR") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].specParameters.boxcarSmoothing = (short) atoi(tok);
            }
            else if(strcmp(tok,"ABSORBANCE_WAVELENGTHS") == 0){
                wavelengthCount = 0;
                tok = strtok(NULL,",\n");
                while(tok != NULL){
                    config[specIndex].waveParameters.absorbingWavelengths[wavelengthCount] = atof(tok); 
                    wavelengthCount++;
                    tok = strtok(NULL,",\n");
                }
                config[specIndex].waveParameters.absorbingWavelengthCount = wavelengthCount; 
            }
           else if(strcmp(tok,"NON_ABSORBING_WAVELENGTH") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.nonAbsorbingWavelength = atof(tok);
            }

            else if(strcmp(tok,"ANALYTE") == 0){
                tok = strtok(NULL,"\n");
                strncpy(config[specIndex].waveParameters.analyteName,tok,MAX_ANA_NAME-1);
                config[specIndex].waveParameters.analyteName[MAX_ANA_NAME] = '\0';
            }
            else if(strcmp(tok,"UNITS") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.units = (char)atoi(tok);
            }
            else if(strcmp(tok,"TEMP") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.temperature = atof(tok);
            }
            else if(strcmp(tok,"CTS1") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.CtS1 = atof(tok);
            }
            else if(strcmp(tok,"PCO2S1") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.pCO2S1 = atof(tok);
            }
            else if(strcmp(tok,"PCO2S2") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.pCO2S2 = atof(tok);
            }
            else if(strcmp(tok,"SYSMEASUREMODE") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.systemMeasureMode = (unsigned char)atoi(tok);
            }
            else if(strcmp(tok,"CMEASUREMODE") == 0){
                tok = strtok(NULL,"\n");
                config[specIndex].waveParameters.cMeasureMode = (unsigned char)atoi(tok);
            }
            else if(strcmp(tok,"SLOPE") == 0){
                tok = strtok(NULL,",\n");
                i=0;
                while(tok != NULL && i < MAX_ABS_WAVES){
                    config[specIndex].waveParameters.slope[wavelengthCount] = atof(tok);
                    tok = strtok(NULL,",\n");
                }
            }
            else if(strcmp(tok,"INTERCEPT") == 0){
                tok = strtok(NULL,",\n");
                i=0;
                while(tok != NULL && i < MAX_ABS_WAVES){
                    config[specIndex].waveParameters.intercept[wavelengthCount] = atof(tok);
                    tok = strtok(NULL,",\n");
                }
            }
            else{ // Unrecognized spectrometer parameter, skip this line
                syslog(LOG_DAEMON|LOG_INFO,"Skipped Unrecognized Config Line: %s",configLine);
                continue;
            }
        }
    }

    fclose(configFile);

    // Apply Mode Setting After All Possible Settings Have Been Read
    setMode(mode);

    return 1;
}

void applyConfig(){
    int i,j;
  
    for(i=0; i < NUM_SPECS; i++)
    {
        setSpecIntegrationTimeinMilli(i,getIntegrationTime(i)); 
        for(j=0; j < getAbsorbingWavelengthCount(i); j++)
        {
            config[i].absCalcParameters.absorbingPixels[j] = calcPixelValueForWavelength(i,config[i].waveParameters.absorbingWavelengths[j]);
        }
        config[i].absCalcParameters.nonAbsorbingPixel = calcPixelValueForWavelength(i,config[i].waveParameters.nonAbsorbingWavelength);
    }
}

void writeConfigFile(){
    int i,j;
    FILE* configFile;

    if(access(CONFIGPATH,R_OK|W_OK)){
        return; // Do not have access to file
    }

    configFile = fopen(CONFIGPATH,"w+");
    if(configFile){
        pthread_mutex_lock(&writeConfigMutex);
        fprintf(configFile,"# System Parameters\n");
        fprintf(configFile,"SYS.MODE=%s\n",mode == MANUAL ? "MANUAL" : "PROGRAM");
        for(i=0; i < 2; i++){
            fprintf(configFile,"# Spectrometer %d Parameters\n",i+1);
            fprintf(configFile,"SPEC%d.SERIAL=%s\n",i+1,config[i].serial);
            fprintf(configFile,"SPEC%d.DWELL=%d\n",i+1,config[i].dwell);
            fprintf(configFile,"SPEC%d.INTEGRATION_TIME=%d\n",i+1,config[i].specParameters.integrationTime);
            fprintf(configFile,"SPEC%d.SCANS_PER_SAMPLE=%d\n",i+1,config[i].specParameters.scansPerSample);
            fprintf(configFile,"SPEC%d.BOXCAR=%d\n",i+1,config[i].specParameters.boxcarSmoothing);
            fprintf(configFile,"\n"); 
            fprintf(configFile,"SPEC%d.ABSORBANCE_WAVELENGTHS=",i+1);
            for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
                if(j==0)
                    fprintf(configFile,"%f",config[i].waveParameters.absorbingWavelengths[j]);
                else
                    fprintf(configFile,",%f",config[i].waveParameters.absorbingWavelengths[j]); 
            }
            fprintf(configFile,"\nSPEC%d.NON_ABSORBING_WAVELENGTH=%f\n",i+1,config[i].waveParameters.nonAbsorbingWavelength);
            fprintf(configFile,"\nSPEC%d.ANALYTE=%s\n",i+1,config[i].waveParameters.analyteName);
            fprintf(configFile,"SPEC%d.UNITS=%d\n",i+1,config[i].waveParameters.units);
            fprintf(configFile,"SPEC%d.TEMP=%f\n",i+1,config[i].waveParameters.temperature);
            fprintf(configFile,"SPEC%d.CTS1=%f\n",i+1,config[i].waveParameters.CtS1);
            fprintf(configFile,"SPEC%d.PCO2S1=%f\n",i+1,config[i].waveParameters.pCO2S1);
            fprintf(configFile,"SPEC%d.PCO2S2=%f\n",i+1,config[i].waveParameters.pCO2S2);
            fprintf(configFile,"SPEC%d.SYSMEASUREMODE=%d\n",i+1,config[i].waveParameters.systemMeasureMode);
            fprintf(configFile,"SPEC%d.CMEASUREMODE=%d\n",i+1,config[i].waveParameters.cMeasureMode);
            fprintf(configFile,"SPEC%d.SLOPE=",i+1);
            for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
                if(j==0)
                    fprintf(configFile,"%f",config[i].waveParameters.slope[j]);
                else
                    fprintf(configFile,",%f",config[i].waveParameters.slope[j]);
            } 
            fprintf(configFile,"\n");
            fprintf(configFile,"SPEC%d.INTERCEPT=",i+1);
            for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
                if(j==0)
                    fprintf(configFile,"%f",config[i].waveParameters.intercept[j]);
                else
                    fprintf(configFile,",%f",config[i].waveParameters.intercept[j]);
            }
            fprintf(configFile,"\n");
            fprintf(configFile,"\n");
        }
        fclose(configFile);
        pthread_mutex_unlock(&writeConfigMutex);

        // If DB is open, log the configuration change
        writeConfigToDB();

    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Cannot open config file @: %s.",CONFIGPATH);
    }
}

// Set Methods
void setMode(systemMode sysMode){
    mode = sysMode;

    if(mode == PROGRAM){
        executeMethodFile();
    } else if (mode == MANUAL){
        terminateMethodFile();
    }
}

char setSpectrometerParameters(int specIndex,unsigned short newIntTime,unsigned short newScansPerSample, unsigned short newBoxcarSmoothing){
    config[specIndex].specParameters.integrationTime = newIntTime;
    setSpecIntegrationTimeinMilli(specIndex,config[specIndex].specParameters.integrationTime);
    config[specIndex].specParameters.scansPerSample = newScansPerSample;
    config[specIndex].specParameters.boxcarSmoothing = newBoxcarSmoothing;
}

char setComputationData(int specIndex, char* newAnalyteName, unsigned char newUnits, unsigned char newAbsWaveCount, float* newAbsWaves, float newNonAbsWave){
    int i;

    strncpy(config[specIndex].waveParameters.analyteName,newAnalyteName,MAX_ANA_NAME-1);
    config[specIndex].waveParameters.units = newUnits;
    config[specIndex].waveParameters.absorbingWavelengthCount = newAbsWaveCount;
    for(i=0; i < newAbsWaveCount; i++){
        config[specIndex].waveParameters.absorbingWavelengths[i] = newAbsWaves[i];
        config[specIndex].absCalcParameters.absorbingPixels[i] = calcPixelValueForWavelength(specIndex,newAbsWaves[i]);
    }
    config[specIndex].waveParameters.nonAbsorbingWavelength = newNonAbsWave;    
    config[specIndex].absCalcParameters.nonAbsorbingPixel = calcPixelValueForWavelength(specIndex,newNonAbsWave);
    applyConfig();
    writeConfigFile();
}

void setAbsorbanceWavelengths(int specIndex, unsigned char newAbsWaveCount,float* newAbsWaves){
    int i;

    config[specIndex].waveParameters.absorbingWavelengthCount = newAbsWaveCount;
    for(i=0; i < newAbsWaveCount; i++){
        config[specIndex].waveParameters.absorbingWavelengths[i] = newAbsWaves[i];
        config[specIndex].absCalcParameters.absorbingPixels[i] = calcPixelValueForWavelength(specIndex,newAbsWaves[i]);
    }
    applyConfig();
    writeConfigFile();
}

void setNonAbsorbingWavelengths(int specIndex, float newNonAbsWave){
    config[specIndex].waveParameters.nonAbsorbingWavelength = newNonAbsWave;
    config[specIndex].absCalcParameters.nonAbsorbingPixel = calcPixelValueForWavelength(specIndex,newNonAbsWave);
    applyConfig();
    writeConfigFile();
}

void setDwell(int specIndex,uint16_t dwell){
    if(specIndex < NUM_SPECS){
        config[specIndex].dwell = dwell;
        writeConfigFile();
    }
}

void setComputationDataBytes(int specIndex,unsigned char* bytes){
    memcpy(&(config[specIndex].waveParameters),bytes,sizeof(wavelengthParameters));
    applyConfig();
    writeConfigFile();
}

void setSlopeIntercept(int specIndex, float* slopeInterceptPairs){
    int i=0;
    for(i=0; i < MAX_ABS_WAVES; i++){
        config[specIndex].waveParameters.slope[i] = slopeInterceptPairs[i*2];
        config[specIndex].waveParameters.intercept[i] = slopeInterceptPairs[i*2+1];
    }
    applyConfig();
    writeConfigFile();
}

spectrometerParameters* getSpecParameters(int specIndex){
    return &(config[specIndex].specParameters);
}

wavelengthParameters* getWaveParameters(int specIndex){
    return &(config[specIndex].waveParameters);
}

// Get Methods
specConfig* getConfigCopy(int specIndex){
    specConfig* retVal = NULL;
	
    if(specIndex < NUM_SPECS){
        retVal = malloc(sizeof(specConfig));
        if(retVal){
            pthread_mutex_lock(&writeConfigMutex);
            memcpy(retVal,&config[specIndex],sizeof(specConfig));
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to allocate enough memory to copy spectrometer #%d config.",specIndex);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Out of Bounds Spectrometer Index (%d) Passed to getConfigCopy.",specIndex);
    }
}

// This method is very simple at the moment, but is here just in case spec config becomes any more complicated
void freeSpecConfig(specConfig* config){
    free(config);
}

systemMode getMode(){
    systemMode retVal = mode; // Make a copy to return
    return retVal;
}
 
char* getSerialNumber(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].serial;
    else
        return NULL;
}
unsigned short getIntegrationTime(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].specParameters.integrationTime;
    else
        return -1;
}
unsigned short getScansPerSample(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].specParameters.scansPerSample;
    else
        return -1;
}
unsigned short getBoxcarSmoothing(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].specParameters.boxcarSmoothing;
    else
        return -1;
}
unsigned char getAbsorbingWavelengthCount(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].waveParameters.absorbingWavelengthCount;
    else
        return -1;
}
float* getAbsorbingWavelengths(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].waveParameters.absorbingWavelengths;
    else
        return NULL;
}
float getNonAbsorbingWavelength(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].waveParameters.nonAbsorbingWavelength;
    else
        return -1;
}

unsigned short* getAbsorbancePixels(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].absCalcParameters.absorbingPixels;
    else
        return NULL;
}

unsigned short getNonAbsorbancePixel(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].absCalcParameters.nonAbsorbingPixel;
    else
        return -1;
}

uint16_t getDwell(int specIndex){
    if(specIndex < NUM_SPECS)
        return config[specIndex].dwell;
    else
        return -1;
}

float* getSlopes(int specIndex){
    size_t arraySize = sizeof(float) * getAbsorbingWavelengthCount(specIndex);
    float* slopes = malloc(arraySize);
    memcpy(slopes,config[specIndex].waveParameters.slope,arraySize);
    return slopes;
}

float* getIntercepts(int specIndex){
    size_t arraySize = sizeof(float) * getAbsorbingWavelengthCount(specIndex);
    float* intercepts = malloc(arraySize);
    memcpy(intercepts,config[specIndex].waveParameters.intercept,arraySize);
    return intercepts;
}

// Config Debug Methods
void logConfig(){
    int i,j;

    syslog(LOG_DAEMON|LOG_INFO,"System Mode: %s",mode == MANUAL ? "MANUAL" : "PROGRAM");

    for(i=0; i < 2; i++){
        syslog(LOG_DAEMON|LOG_INFO,"Spectrometer %i Configuration",i+1);
        syslog(LOG_DAEMON|LOG_INFO,"=============================");
        syslog(LOG_DAEMON|LOG_INFO,"Serial Number: %s",config[i].serial);
        syslog(LOG_DAEMON|LOG_INFO,"Dwell Time: %d",config[i].dwell);
        syslog(LOG_DAEMON|LOG_INFO,"Integration Time: %d",config[i].specParameters.integrationTime);
        syslog(LOG_DAEMON|LOG_INFO,"Scans Per Sample: %d",config[i].specParameters.scansPerSample);
        syslog(LOG_DAEMON|LOG_INFO,"Boxcar Smoothing: %d",config[i].specParameters.boxcarSmoothing);
        syslog(LOG_DAEMON|LOG_INFO,"Absorbing Wavelength Count: %d",config[i].waveParameters.absorbingWavelengthCount);
        for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++){
            syslog(LOG_DAEMON|LOG_INFO,"Absorbing Wavelength %i: %f",j+1,config[i].waveParameters.absorbingWavelengths[j]);
        }
        syslog(LOG_DAEMON|LOG_INFO,"Non Absorbing Wavelength: %f",config[i].waveParameters.nonAbsorbingWavelength); 
        syslog(LOG_DAEMON|LOG_INFO,"Analyte Name: %s",config[i].waveParameters.analyteName);
        syslog(LOG_DAEMON|LOG_INFO,"Units #: %d",config[i].waveParameters.units);
        syslog(LOG_DAEMON|LOG_INFO,"System Measure Mode: %d",config[i].waveParameters.systemMeasureMode);
        syslog(LOG_DAEMON|LOG_INFO,"Carbon Measure Mode: %d",config[i].waveParameters.cMeasureMode);
        syslog(LOG_DAEMON|LOG_INFO,"Temperature: %f",config[i].waveParameters.temperature);
        syslog(LOG_DAEMON|LOG_INFO,"CtS1: %f",config[i].waveParameters.CtS1);
        syslog(LOG_DAEMON|LOG_INFO,"pCO2S1: %f",config[i].waveParameters.pCO2S1);
        syslog(LOG_DAEMON|LOG_INFO,"pCO2S2: %f",config[i].waveParameters.pCO2S2);
        for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++)
            syslog(LOG_DAEMON|LOG_INFO,"Slope %i: %f",j+1,config[i].waveParameters.slope[j]);
        for(j=0; j < config[i].waveParameters.absorbingWavelengthCount; j++)
            syslog(LOG_DAEMON|LOG_INFO,"Intercept %i: %f",j+1,config[i].waveParameters.intercept[j]);
    }
}


#include "parseGUI.h"

static unsigned short multiBufferCommand = 0;

void parseWaveSetCommand(char* waveBytes){
    int specID;
    char newAnaName[MAX_ANA_NAME];
    unsigned char newUnits;
    unsigned char absWaveCount;
    float absWaves[9];
    float nonAbsWave;

    specID = (int)waveBytes[0];
    
    setComputationDataBytes(specID,waveBytes+1);
}

void parseParameterBytes(char* parameterBytes){
    int specID;
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;

    specID = (int)parameterBytes[0];
    memcpy(&integrationTime,parameterBytes+1,2);
    memcpy(&scansPerSample,parameterBytes+3,2);
    memcpy(&boxcarSmoothing,parameterBytes+5,2);
    
    setSpectrometerParameters(specID,integrationTime,scansPerSample,boxcarSmoothing);
}

GUIresponse* parseGUI(char* command){
    GUIresponse* response = NULL;
    specSample* sample = NULL;
    int i;
    unsigned short switchVar;

    if(multiBufferCommand){
        switchVar = multiBufferCommand;
    }else{
        switchVar = command[0];
    }

    switch(switchVar){
        case STM:
            break;
        case RTM:
            break;
        case RSD:
            break;
        case RSF:
            break;
        case PMW:
            break;
        case PMC:
            break;
        case HTP:
            break;
        case HTC:
            break;
        case LTC:
            break;
        case SVS:
            break;

        case SSP:
            syslog(LOG_DAEMON||LOG_INFO,"Setting Spectrometer Parameters...");
            parseParameterBytes(command+1);
            syslog(LOG_DAEMON||LOG_INFO,"Spectrometer Parameters Set.");
            break;
        case RFS:
            syslog(LOG_DAEMON||LOG_INFO,"Retrieving wavelength sample from USB4000");
            response = malloc(sizeof(GUIresponse));
            sample = getSpecSample(command[1],getScansPerSample(command[1]),100);
            // Copy the Pointer to the Pixels Array
            response->response = (void *)sample->pixels;
            response->length = sizeof(float)*3840;

            // Remove Old Sample Structure
            free(sample);

            syslog(LOG_DAEMON||LOG_INFO,"Retrieved wavelength. Returning");
            break;

        case RCD:
            syslog(LOG_DAEMON||LOG_INFO,"Retrieving calibration coefficients from USB4000");
            response = malloc(sizeof(GUIresponse));
            response->response = (void *)getCalCos(command[1]);
            response->length = sizeof(calibrationCoefficients);
            syslog(LOG_DAEMON||LOG_INFO,"Retrieved calibration coefficients. Returning.");
            break;

        case RSP:
            syslog(LOG_DAEMON||LOG_INFO,"Sending Spectrometer Parameters.");
            response = malloc(sizeof(GUIresponse));
            response->response = malloc(sizeof(spectrometerParameters));
            memcpy(response->response,(void *)getSpecParameters(command[1]),sizeof(spectrometerParameters));
            response->length = sizeof(spectrometerParameters); 
            break;
        case EXM:
            break;
        case TRM:
            break;
        case LRM:
            syslog(LOG_DAEMON||LOG_INFO,"Loading Method.");
            response = receiveMethodFile(command);
            // If you have received the whole file, response will not be null.  Else, make a note that there are still more buffers expected.
            if(response){
                multiBufferCommand = 0;
            }else{
                multiBufferCommand = LRM;
            }
            break;
        case LMT:
            syslog(LOG_DAEMON||LOG_INFO,"Loading Method List.");
            response = getMethodFileList();
            break; 
        case RMF:
            syslog(LOG_DAEMON||LOG_INFO,"Reading Method File.");
            response = readMethodFile(command); 
            break;
        case SMF:
            syslog(LOG_DAEMON||LOG_INFO,"Setting Active Method File.");
            response = NULL;
            break;
        case DLM:
            break;
        case SVC:
            syslog(LOG_DAEMON||LOG_INFO,"Saving Configuration.");
            writeConfigFile();
            syslog(LOG_DAEMON||LOG_INFO,"Configuration Saved.");
            response = malloc(sizeof(GUIresponse));
            response->response = malloc(sizeof(char));
            ((char *)response->response)[0] = SVC;
            response->length = 1;
            break;
        case SCP:
            syslog(LOG_DAEMON||LOG_INFO,"Setting Wavelength Configuration.");
            parseWaveSetCommand(command+1);
            syslog(LOG_DAEMON||LOG_INFO,"Wavelength Configuration Set.");
            break; 
        case SDC:
            break;
        case SDE:
            break;
        case SFR:
            break;
        case SFS:
            break;
        case RCP:
            response = malloc(sizeof(GUIresponse));
            response->response = malloc(sizeof(wavelengthParameters));
            memcpy(response->response,(void *)getWaveParameters(command[1]),sizeof(wavelengthParameters)); 
            response->length = sizeof(wavelengthParameters);
            break;
        case RAD:
            syslog(LOG_DAEMON||LOG_INFO,"Retrieving Absorbance for Specified Wavelengths.");
            response = malloc(sizeof(GUIresponse));
            response->response = getAbsorbance(command[1]);
            response->length = sizeof(float) * (MAX_ABS_WAVES+1);
            syslog(LOG_DAEMON||LOG_INFO,"Absorbance for Specified Wavelengths Retrieved.");
            break;
        case RAS:
            syslog(LOG_DAEMON||LOG_INFO,"Retrieving Absorbance Spectrum...");
            response = malloc(sizeof(GUIresponse));
            response->response = getAbsorbanceSpectrum(command[1]);
            response->length = sizeof(float) * 3840;
            syslog(LOG_DAEMON||LOG_INFO,"Absorbance Spectrum Retrieved.");
            break;
        case RCC:
            break;
        case RDS:
            syslog(LOG_DAEMON||LOG_INFO,"Recording Dark Sample...");
            recordDarkSample(command[1],getScansPerSample(command[1]),100);
            response = malloc(sizeof(GUIresponse));
            response->response = malloc(sizeof(char));
            ((char *)response->response)[0] = RDS;
            response->length = 1;
            syslog(LOG_DAEMON||LOG_INFO,"Dark Sample Recorded.");
            break;
        case RRS:
            syslog(LOG_DAEMON||LOG_INFO,"Recording Reference Sample...");
            recordRefSample(command[1],getScansPerSample(command[1]),100);
            response = malloc(sizeof(GUIresponse));
            response->response = malloc(sizeof(char));
            ((char *)response->response)[0] = RRS;
            response->length = 1;
            syslog(LOG_DAEMON||LOG_INFO,"Reference Sample Recorded.");
            break;
        case RSS:
            syslog(LOG_DAEMON||LOG_INFO,"Recording Sample.");
            recordSpecSample(command[1],getScansPerSample(command[1]),100);
            syslog(LOG_DAEMON||LOG_INFO,"Sample Recorded");
            break;
        case RMW:
            break;
        case RPS:
            break;
        case RTP:
            break;
        case RHS:
            break;
        case RLS:
            break;
        case RVS:
            break;
        default:
            syslog(LOG_DAEMON||LOG_INFO,"Unknown command sent starting with: %02x",command[0]);
            break;
    }
    return response;
}

GUIresponse* createResponse(unsigned int length, void* data){
    GUIresponse* response = malloc(sizeof(GUIresponse));
    response->length = length;
    response->response = data;
    return response;
}

GUIresponse* createResponseString(char* string){
    GUIresponse* response = malloc(sizeof(GUIresponse));
    response->length = strlen(string);
    response->response = malloc(sizeof(char)*(response->length+1));
    strcpy((char *)response->response,string);
    return response;
}

void freeResponse(GUIresponse* response){
    if(response->response){
	free(response->response);
    }
    if(response){
        free(response);
    }
}

#include "parseGUI.h"

void parseWaveSetCommand(char* waveBytes){
    int specID;
    unsigned char absWaveCount;
    float absWaves[9];
    float nonAbsWave;

    specID = (int)waveBytes[0];
    absWaveCount = (unsigned char)waveBytes[1];
    // Add +3 to skip reserved bytes
    memcpy(absWaves,waveBytes+5,sizeof(float)*9);
    memcpy(&nonAbsWave,waveBytes+41,sizeof(float));
    
    setComputationData(specID,absWaveCount,absWaves,nonAbsWave); 
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

    switch(command[0]){
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
        case LDM:
            break;
        case RDM:
            break;
        case SMR:
            break;
        case SMF:
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

void freeResponse(GUIresponse* response){
    if(response->response){
	free(response->response);
    }
    if(response){
        free(response);
    }
}

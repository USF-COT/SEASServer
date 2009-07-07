#include "parseGUI.h"

void parseParameterBytes(char* parameterBytes){
    int specID;
    unsigned short integrationTime;
    unsigned short scansPerSample;
    unsigned short boxcarSmoothing;

    specID = (int)parameterBytes[0];
    memcpy(&integrationTime,parameterBytes+1,2);
    memcpy(&scansPerSample,parameterBytes+3,2);
    memcpy(&boxcarSmoothing,parameterBytes+5,2);
    
    // Set Spectrometer Integration Time Immediately
    setSpecIntegrationTime(specID, integrationTime);
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
        case DSF:
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
            response->response = sample->pixels;
            response->length = sizeof(float)*3840;
            syslog(LOG_DAEMON||LOG_INFO,"Retrieved wavelength. Returning");
            break;

        case RCD:
            syslog(LOG_DAEMON||LOG_INFO,"Retrieving calibration coefficients from USB4000");
            response = malloc(sizeof(GUIresponse));
            response->response = (void *)getCalCos(command[1]);
            response->length = sizeof(calibrationCoefficients);
            syslog(LOG_DAEMON||LOG_INFO,"Retrieved calibration coefficients. Returning.");
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
        case SDC:
            break;
        case SDE:
            break;
        case SFR:
            break;
        case SFS:
            break;
        case SWL:
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
    }
    return response;
}

void freeResponse(GUIresponse* response){
    if(response){
        free(response);
    }
}

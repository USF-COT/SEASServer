#include "parseGUI.h"

GUIresponse* parseGUI(char* command){
    GUIresponse* response = NULL;

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
        case RFS:
            response->response = (void *)getSpecSample(0,10,100);
            response->length = sizeof(specSample);
            break;
        case RCD:
            response->response = (void *)getCalCos(0);
            response->length = sizeof(calibrationCoefficients);
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
        free(response->response);
        free(response);
    }
}

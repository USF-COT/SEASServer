#include "SEASPeripheralCommands.h"

static periphStatuses_s status;
static const uint16_t pumpLONGPIO[MAX_NUM_PUMP_LON_NODES] = {PUMP_A_GPIO,PUMP_B_GPIO,PUMP_C_GPIO};

// Power Management Mutex
static pthread_mutex_t periphPMMutex = PTHREAD_MUTEX_INITIALIZER;

void setupGPIO(uint16_t GPIO){
    char sysCmd[128];

    sysCmd[0] = '\0';
    sprintf(sysCmd,GPIO_EXP_FMT,GPIO);
    system(sysCmd);

    sysCmd[0] = '\0';
    sprintf(sysCmd,GPIO_OUT_FMT,GPIO);
    system(sysCmd);
}

void switchGPIO(BOOL powerOn, uint16_t GPIO){
    char sysCmd[128];
    
    sysCmd[0] = '\0';
    sprintf(sysCmd,GPIO_VAL_FMT,!powerOn,GPIO);
    if(GPIO == LON_INT_GPIO) syslog(LOG_DAEMON|LOG_INFO,"Following System Command Called on Gateway: %s",sysCmd);
    system(sysCmd);
}

// Called in main.c to initialize the GPIO's to their default startup state
void initPeripherals(){
    int i,j;
    uint8_t pumpID;

    pthread_mutex_lock(&periphPMMutex);
    // CTD On
    status.CTDStatus = TRUE;
    setupGPIO(CTD_NODE_GPIO);
    switchGPIO(status.CTDStatus,CTD_NODE_GPIO);

    // LON Gateway On
    status.LONHead = TRUE;
    setupGPIO(LON_INT_GPIO);
    switchGPIO(status.LONHead,LON_INT_GPIO);

    // Pumps Off
    for(i=0; i < MAX_NUM_PUMP_LON_NODES; i++){
        for(j=0; j < NUM_PUMPS_PER_LON; j++){
            pumpID = NUM_PUMPS_PER_LON*i+j;
            status.pumpStatus[pumpID] = FALSE;
            status.pumpPercents[pumpID] = getBenchPumpRPM(pumpID);
        }
        setupGPIO(pumpLONGPIO[i]);
        switchGPIO(FALSE,pumpLONGPIO[i]);
    }

    // Heater Off
    status.heaterStatus = FALSE;
    status.heaterSetTemp = getBenchHeaterSetPoint();
    setupGPIO(HEAT_GPIO);
    switchGPIO(status.heaterStatus,HEAT_GPIO);

    // Spare Off
    setupGPIO(SPARE_GPIO);
    switchGPIO(FALSE,SPARE_GPIO);
    pthread_mutex_unlock(&periphPMMutex);
}

BOOL getPumpLONState(unsigned char pumpLONNode){
    int i;
    BOOL LONState = FALSE;

    if(pumpLONNode < MAX_NUM_PUMP_LON_NODES){
        for(i=pumpLONNode*NUM_PUMPS_PER_LON; i <= pumpLONNode*NUM_PUMPS_PER_LON+1; i++){
            LONState |= status.pumpStatus[i];
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"pumpLONNode index (%d) passed to getPumpLONState out of bounds.",pumpLONNode);
    }
    syslog(LOG_DAEMON|LOG_INFO,"pumpLONNode (%d) found to be %s.",pumpLONNode,LONState ? "ON" : "OFF");
    return LONState;
}

// LON Node Switch Commands
void switchPumpNode(unsigned char pumpID,BOOL powerOn){
    unsigned char pumpLONNode;

    int i;
    BOOL currLONState = FALSE;
    BOOL nextLONState = FALSE;
    BOOL switchedOn = FALSE;

    pumpID = pumpID - 1; // Decrement 1 based index
    pumpLONNode = (unsigned char)floor(pumpID/NUM_PUMPS_PER_LON);

    if(pumpID < MAX_NUM_PUMPS && pumpLONNode < MAX_NUM_PUMP_LON_NODES){
        pthread_mutex_lock(&periphPMMutex);
        currLONState = getPumpLONState(pumpLONNode);
        status.pumpStatus[pumpID] = powerOn;
        nextLONState = getPumpLONState(pumpLONNode);

        if(currLONState == FALSE && nextLONState == TRUE){
            syslog(LOG_DAEMON|LOG_INFO,"Switching pump LON node %d ON.", pumpLONNode);
            switchGPIO(TRUE,pumpLONGPIO[pumpLONNode]); 
            switchedOn = TRUE;
        } else if(currLONState == TRUE && nextLONState == FALSE){
            syslog(LOG_DAEMON|LOG_INFO,"Switching pump LON node %d OFF.", pumpLONNode);
            switchGPIO(FALSE,pumpLONGPIO[pumpLONNode]);
        }
        pthread_mutex_unlock(&periphPMMutex);
        
        if(switchedOn) sleep(1);  // Done outside the mutex so that it is not held during sleep.
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Pump ID Out of Range.  Calculated pumpLONNode=%d",pumpLONNode);
    }
}

void switchHeaterNode(BOOL powerOn){
    BOOL switchedOn = FALSE;

    pthread_mutex_lock(&periphPMMutex);
    syslog(LOG_DAEMON|LOG_INFO,"Switching heater LON node %s.",powerOn ? "ON":"OFF");
    if(!status.heaterStatus && powerOn){ // If heater is currently off and powerOn is true, switch the heater GPIO on
        switchGPIO(TRUE,HEAT_GPIO);
        switchedOn = TRUE;
    } else if (status.heaterStatus && !powerOn){ // If heater is currently on and powerOn is false, switch the heater GPIO off
        switchGPIO(FALSE,HEAT_GPIO);
    }
    pthread_mutex_unlock(&periphPMMutex);

    status.heaterStatus = powerOn; // Log this change
    if(switchedOn) sleep(1);  // Done outside the mutex so that it is not held during sleep.
}

// Base Functions
void pumpOn(unsigned char pumpID){
    LONresponse_s* response;
    unsigned char data[2] = {pumpID,ENA};

    if(pumpID < (MAX_NUM_PUMPS + 1)){
        syslog(LOG_DAEMON|LOG_INFO,"Enabling pump %d.",pumpID);
        switchPumpNode(pumpID,TRUE);

        setPumpPercent(pumpID,status.pumpPercents[pumpID-1]);

        // Send the LON Node the command to turn this pump on
        response = sendLONCommand(PMP,PWR,2,data);
        if(response->deviceID == ACK){
            syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d enabled.  LON sent ACK.",pumpID);
        } else {
            syslog(LOG_DAEMON|LOG_ERR, "ERROR: Pump %d not enabled.  LON sent NAK.",pumpID);
        }
        freeLONResponse(response);
    }
}
void pumpOff(unsigned char pumpID){
    LONresponse_s* response;
    unsigned char data[2] = {pumpID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Disabling pump %d.",pumpID);
    response = sendLONCommand(PMP,PWR,2,data);
    switchPumpNode(pumpID,FALSE);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d disabled.  LON sent ACK.",pumpID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d not diabled.  LON sent NAK.",pumpID);
    }
    freeLONResponse(response);
}

void setPumpPercent(unsigned char pumpID, uint16_t percent){
    LONresponse_s* response;
    unsigned char data[3];

    BOOL isOn = getPumpLONState((unsigned char)(pumpID-1)/NUM_PUMPS_PER_LON);

    if(pumpID < (MAX_NUM_PUMPS+1) && percent <= MAX_PUMP_PERCENT){
        status.pumpPercents[pumpID-1] = percent;

        if(isOn){
            data[0] = pumpID;
            data[1] = (unsigned char)((percent & 0xFF00) >> 8);
            data[2] = (unsigned char)(percent & 0xFF);

            syslog(LOG_DAEMON|LOG_INFO,"Setting pump %d percent to: %d/%d.",pumpID,percent,MAX_PUMP_PERCENT);

            response = sendLONCommand(PMP,PWL,3,data);
            if(response->deviceID == ACK){
                syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Pump %d percent set to: %d/%d.",pumpID,percent,MAX_PUMP_PERCENT);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d percent NOT set to: %d/%d.",pumpID,percent,MAX_PUMP_PERCENT);
            }
            freeLONResponse(response);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Invalid Pump ID (%d) or out of range (%d) percent (%d) passed to setPumpPercent.",pumpID,MAX_PUMP_PERCENT,percent);
    }
}

void lampOn(){
    LONresponse_s* response;
    unsigned char data[1] = {ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp on.");    
    response = sendLONCommand(LTE,PWR,1,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned on.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned on.");
    }
    freeLONResponse(response);
}

void lampOff(){
    LONresponse_s* response;
    unsigned char data[1] = {DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Setting lamp off.");
    response = sendLONCommand(LTE,PWR,1,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Lamp turned off.");
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Lamp not turned off.");
    }
    freeLONResponse(response);
}

void heaterOn(unsigned char heaterID){
    LONresponse_s* response;
    unsigned char data[2] = {heaterID,ENA};

    syslog(LOG_DAEMON|LOG_INFO,"Turning on Heater %d.",heaterID);
    switchHeaterNode(TRUE);
    setHeaterTemp(heaterID,status.heaterSetTemp);
    response = sendLONCommand(HTR,PWR,2,data);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned on.",heaterID); 
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned on.",heaterID);
    }
    freeLONResponse(response);
}

void heaterOff(unsigned char heaterID){
    LONresponse_s* response;
    unsigned char data[2] = {heaterID,DIS};

    syslog(LOG_DAEMON|LOG_INFO,"Turning off Heater %d.",heaterID);
    response = sendLONCommand(HTR,PWR,2,data);
    switchHeaterNode(FALSE);
    if(response->deviceID == ACK){
        syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d turned off.",heaterID);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d not turned off.",heaterID);
    }
    freeLONResponse(response);
}

void setHeaterTemp(unsigned char heaterID, float temperature){
    LONresponse_s* response;
    unsigned char data[5];

    status.heaterSetTemp = temperature;
    
    if(status.heaterStatus){
        data[0] = heaterID;
        copyReverseBytes(data+1,(void*)&temperature,4);
    
        syslog(LOG_DAEMON|LOG_INFO,"Setting heater %d temperature to: %f.",heaterID,temperature);
        switchHeaterNode(TRUE);
        response = sendLONCommand(HTR,TMP,5,data);
        if(response->deviceID == ACK){
            syslog(LOG_DAEMON|LOG_INFO,"SUCCESS: Heater %d temperature set to: %f.",heaterID,temperature);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater %d temperature NOT set to: %f.",heaterID,temperature);
        }
        freeLONResponse(response);
    }
}

// Status Functions

pumpStatus_s* getPumpStatus(unsigned char pumpID){
    pumpStatus_s* status = NULL;
    unsigned char sendData[1] = {pumpID};
    LONresponse_s* response = sendLONCommand(PMP,STS,1,sendData);
    if(response){
        if(response->data && response->deviceID == PMP){
            status = malloc(sizeof(pumpStatus_s));
            status->pumpID = response->data[0];
            status->power = response->data[1];
            copyReverseBytes(&(status->percent),response->data+2,4);        
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to retrieve pump %d status.",pumpID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Did not receive a response from LON for pump %d status command.",pumpID);
    }
    return status;
}

float getHeaterCurrentTemperature(unsigned char heaterID){
    float currentTemp = -1;
    unsigned char sendData[1] = {heaterID};
    LONresponse_s* response = sendLONCommand(HTR,RCT,1,sendData);
    if(response){
        if(response->data && response->deviceID == HTR && response->data[0] == heaterID){
            copyReverseBytes(&currentTemp,response->data+1,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to retrieve current temperature from heater %d.",heaterID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for current temperature from heater %d.",heaterID);
    }
    return currentTemp;
}

heaterStatus_s* getHeaterStatus(unsigned char heaterID){
    heaterStatus_s* status = NULL;
    unsigned char sendData[1] = {heaterID};
    LONresponse_s* response = sendLONCommand(HTR,STS,1,sendData);
    if(response){
        if(response->deviceID == HTR && response->data && response->data[0] == heaterID){
            status = malloc(sizeof(heaterStatus_s));
            status->heaterID = heaterID;
            status->power = response->data[1];
            copyReverseBytes(&(status->setTemperature),response->data+2,4);
            copyReverseBytes(&(status->currentTemperature),response->data+6,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for status of heater %d.",heaterID);
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for status from heater %d",heaterID);
    }
    return status;
}

unsigned char getLampStatus(){
    unsigned char status = DIS;
    LONresponse_s* response = sendLONCommand(LTE,STS,0,NULL);
    if(response){
        if(response->data && response->deviceID == LTE){
            status = response->data[0];
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for status of lamp.");
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for status of lamp.");
    }
    return status;
}

CTDreadings_s* getCTDValues(){
    CTDreadings_s* readings = NULL;
    LONresponse_s* response = sendLONCommand(CTD,CTR,0,NULL);
    if(response){
        if(response->data && response->deviceID == CTD){
            readings = malloc(sizeof(CTDreadings_s));
            copyReverseBytes(&(readings->conductivity),response->data,4);
            copyReverseBytes(&(readings->temperature),response->data+4,4);
            copyReverseBytes(&(readings->pressure),response->data+8,4);
            copyReverseBytes(&(readings->soundVelocity),response->data+12,4);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Incorrect LON response received when looking for CTD readings.");
        }
        freeLONResponse(response);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: No LON response received when looking for battery voltage.");
    }
    return readings;
}

// GUI Protocol Wrappers
void receiveSetPumpControl(int connection, char* command){
    if(command[0] == PMC){
        syslog(LOG_DAEMON|LOG_INFO,"Switching pump %d %s.",command[1],command[2] == 1 ? "ON" : "OFF");
        if(command[2] == 1){
            pumpOn(command[1]);
        } else {
            pumpOff(command[1]);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetPumpControl method.");
    }
}

void receiveSetPumpPercent(int connection, char* command){
    unsigned char pumpID;
    unsigned int percent;

    if(command[0] == PMW){
        pumpID = command[1];
        percent = (command[3] << 8) + command[2]; // Received in little endian
        percent = percent * 10;
        syslog(LOG_DAEMON|LOG_INFO,"Setting pump %d Power to: %d",pumpID,percent);
        setPumpPercent(pumpID,percent);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetPumpPercent method.");
    }
}

void receiveSetLampControl(int connection, char* command){
    if(command[0] == LTC){
        syslog(LOG_DAEMON|LOG_INFO,"Turning lamp %s.",command[1] == 1 ? "ON" : "OFF");
        if(command[1] == 1){
            lampOn();
        } else {
            lampOff();
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetLampControl method.");
    }
}

void receiveSetHeaterControl(int connection, char* command){
    unsigned char heaterID;

    if(command[0] == HTC){
        heaterID = 1;
        syslog(LOG_DAEMON|LOG_INFO,"Turning heater %s.",command[1] == 1 ? "ON" : "OFF");
        if(command[1] == 1){
            heaterOn(heaterID);
        } else {
            heaterOff(heaterID);
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetHeaterControl method.");
    }
}

void receiveSetHeaterTemp(int connection, char* command){
    unsigned char heaterID;
    float temperature;

    if(command[0] == HTP){
        heaterID = 1;
        memcpy(&temperature,command+1,4);
        syslog(LOG_DAEMON|LOG_INFO,"Setting heater temperature to %f.",temperature);
        setHeaterTemp(heaterID,temperature);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveSetHeaterControl method.");
    }
}


void receiveGetPumpStatus(int connection, char* command){
    pumpStatus_s* status = NULL;
    unsigned char sendBuffer[7];
    syslog(LOG_DAEMON|LOG_INFO,"Getting pump %d status.",command[1]);

    if(command[0] == RPS){
        status = getPumpStatus(command[1]);
        if(status){
            syslog(LOG_DAEMON|LOG_INFO,"Pump %d status retrieved.  Pump is %s. RPM Percent is %d.",status->pumpID,status->power == ENA ? "ON" : "OFF",status->percent);
            sendBuffer[0] = RPS;
            sendBuffer[1] = status->pumpID;
            sendBuffer[2] = status->power == ENA;
            memcpy(sendBuffer+3,&(status->percent),4);
            send(connection,sendBuffer,7,0);
            free(status);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Pump %d status not retrieved from LON.",command[1]);
            sendErrorMessageBack(connection,"ERROR: Pump status not retrieved from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetPumpStatus method.");
    }
}

void receiveGetHeaterStatus(int connection, char* command){
    heaterStatus_s* status = NULL;
    unsigned char sendBuffer[10];
    char* byteTemp;
    syslog(LOG_DAEMON|LOG_INFO,"Getting heater status.");

    if(command[0] == RHS){
        status = getHeaterStatus(1);
        if(status){
            syslog(LOG_DAEMON|LOG_INFO,"Heater status retrieved.  Heater is %s.  Set temperature is %g.  Current temperature is %g.",status->power == ENA ? "ON" : "OFF",status->setTemperature,status->currentTemperature);
            sendBuffer[0] = RHS;
            sendBuffer[1] = status->power == ENA;
            memcpy(sendBuffer+2,&(status->setTemperature),4);
            memcpy(sendBuffer+6,&(status->currentTemperature),4);
            byteTemp = byteArrayToString(sendBuffer, 12);
            syslog(LOG_DAEMON|LOG_INFO,"Responding with bytes: %s",byteTemp);
            free(byteTemp);
            send(connection,sendBuffer,10,0);
            free(status);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Heater status not retrieved from LON.");
            sendErrorMessageBack(connection,"ERROR: Heater status not retrieved from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetHeaterStatus method.");
    }
}

void receiveGetLampStatus(int connection, char* command){
    unsigned char sendBuffer[2];

    syslog(LOG_DAEMON|LOG_INFO,"Getting light status.");
    if(command[0] == RLS){
        sendBuffer[0] = RLS;
        sendBuffer[1] = getLampStatus() == ENA;
        send(connection,sendBuffer,2,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetLampStatus method.");
    }
}

void receiveGetCTDValues(int connection, char* command){
    unsigned char sendBuffer[17];
    CTDreadings_s* readings = NULL;

    if(command[0] == RTD){
        readings = getCTDValues();
        if(readings){
            syslog(LOG_DAEMON|LOG_INFO,"CTD Readings - Cond: %f, Temp: %f, Pres: %f, Sound: %f.",readings->conductivity,readings->temperature,readings->pressure,readings->soundVelocity);
            sendBuffer[0] = RTD;
            memcpy(sendBuffer+1,&(readings->conductivity),4);
            memcpy(sendBuffer+5,&(readings->temperature),4);
            memcpy(sendBuffer+9,&(readings->pressure),4);
            memcpy(sendBuffer+13,&(readings->soundVelocity),4);
            send(connection,sendBuffer,17,0);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Could not read CTD readings from LON.");
            sendErrorMessageBack(connection,"ERROR: Could not read CTD readings from LON.");
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveGetCTDValues method.");
    }
}

void receiveGetTemperatureValue(int connection, char* command){
    unsigned char sendBuffer[5];
    float heaterTemp = 0;
    
    if(command[0] == RHT){
        sendBuffer[0] = RHT;
        heaterTemp = getHeaterCurrentTemperature(1);
        memcpy(sendBuffer+1,&heaterTemp,4);
        syslog(LOG_DAEMON|LOG_INFO,"Sending heater current temperature: %3.5f",heaterTemp);
        send(connection,(void*)sendBuffer,sizeof(unsigned char)*5,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command (0x%02x) sent ot receive GetTemperatureValue.",command[0]);
    }
}

// Method Wrappers
void methodPumpOn(unsigned long argc, void* argv){
    unsigned char pumpID;
    unsigned int percent;
    double* arguments = (double*) argv;

    if(argc != 2){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to methodPumpOn function.");
        return;
    }

    pumpID = (unsigned char) arguments[0];
    percent = (unsigned int) arguments[1]*10;

    pumpOn(pumpID);
    setPumpPercent(pumpID,percent);
}

void methodPumpOff(unsigned long argc, void* argv){
    unsigned char pumpID;
    double* arguments = (double*) argv;

    if(argc != 1){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to methodPumpOff function.");
        return;
    }

    pumpID = (unsigned char) arguments[0];
    pumpOff(pumpID);
}

void methodLampOn(unsigned long argc, void* argv){
    if(argc != 0){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to lampOn function.");
        return;
    }
    lampOn();
}

void methodLampOff(unsigned long argc, void* argv){
    if(argc != 0){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to lampOff function.");
        return;
    }
    lampOff();
}

void methodHeaterOn(unsigned long argc, void* argv){
    float temperature;
    double* arguments = (double*)argv;

    if(argc != 1){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to heaterOn function.");
        return;
    }

    temperature = (float)arguments[0];
    heaterOn(1);
    setHeaterTemp(1,temperature);
}

void methodHeaterOff(unsigned long argc, void* argv){
    if(argc != 0){
        syslog(LOG_DAEMON|LOG_ERR,"Wrong number of arguments passed to heaterOff function.");
        return;
    }

    heaterOff(1);
}


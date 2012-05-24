#include "LONDispatch.h"


static int portID = -1;
static volatile sig_atomic_t dispatching = 0;
static pthread_mutex_t LONportMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t LONdispatchThread;

void *dispatcher(void* blah){
    int readLength=0;
    unsigned char readBuffer[512];

    dispatching = 1;
    while(dispatching){
        // Look for commands sent from the LON to the Gumstix
        pthread_mutex_lock(&LONportMutex);
        readLength = read(portID,readBuffer,512);
        if(readLength > 0){
            // Handle LON Commands Here.  None yet, but planning for future.
        }
        pthread_mutex_unlock(&LONportMutex);

        // Sleep this thread for a second so that the main thread can send commands.
        sleep(2);
    }
    pthread_exit(NULL);
}

int connect(char* port){
    int fd;
    struct termios settings;

    fd = open(port,O_RDWR|O_NOCTTY|O_NONBLOCK);
    if(fd != -1){
        // Get default port settings
        if(tcgetattr(fd,&settings)==0){
            memset(&settings,0,sizeof(struct termios));
            cfmakeraw(&settings);
            // Set speed
            cfsetspeed(&settings,B19200);
            // Set 8 Data Bits
            settings.c_cflag |= (CS8|CREAD|CLOCAL);
            // Ignore Parity (No Parity)
            settings.c_iflag |= IGNPAR;
            // 1 Stop Bit
            settings.c_cflag &= ~CSTOPB;
            // Read returns when 1 byte is ready
            settings.c_cc[VMIN] = 1;
            settings.c_cc[VTIME] = 0;
            // Commit settings to open port
            if(tcsetattr(fd,TCSANOW,&settings) == 0){
                tcgetattr(fd,&settings);
                //syslog(LOG_DAEMON|LOG_INFO,"VMIN=%d, VTIME=%d.",settings.c_cc[VMIN],settings.c_cc[VTIME]);
                return fd;
            }
        }
        // If the port was open, but could not be setup, close it.
        close(fd);
    }
    return -1;
}

void disconnect(int port){
    close(port);
}

int startDispatch(char* port){
    pthread_attr_t LONdispatchAttr;
    
    if(!dispatching){
        portID = connect(port);
        if(portID != -1){
            pthread_attr_init(&LONdispatchAttr);
            pthread_attr_setdetachstate(&LONdispatchAttr, PTHREAD_CREATE_DETACHED);
            pthread_create(&LONdispatchThread,&LONdispatchAttr,dispatcher,NULL);
            return 1; 
        } else {
            return -1;
        }
        return 1;
    } else {
        return 1;
    }
}

void stopDispatch(){
    if(dispatching){
        dispatching = 0;
    }
}

// Returns 1 if a byte is able to be read, or returns -1
short readByte(unsigned char* buffer){
    int bytesRead,tries=0;
    const int MAX_TRIES=3;

    bytesRead = read(portID,buffer,1);

    while(bytesRead <= 0 && tries < MAX_TRIES){
        usleep(30000);
        bytesRead = read(portID,buffer,1);
        tries++;
    }
    
    if(bytesRead > 0){
        return 1;
    } else {
        return -1;
    }
}

unsigned char* NAKresponse(){
    unsigned char* response = (unsigned char*)malloc(sizeof(unsigned char)*2);

    response[0] = NAK;
    response[1] = '\0';
    return response;
}

// Private method meant to clean up code a bit
unsigned char* readLONResponse(){
    int i;
    unsigned int responseLength;
    unsigned char* response;
    unsigned char responseHeader[3];
    unsigned char readBuffer[3];


    // Read Device ID
    if(readByte(readBuffer) == 1){
        if(readBuffer[0] == ACK){
            response = (unsigned char*)malloc(sizeof(unsigned char)*2);
            response[0] = ACK;
            response[1] = '\0';
            return response;
        } else {
            responseHeader[0] = readBuffer[0];
        }
    } else { // Unable to read the device character from the LON Port, return error code NAK
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read device ID character from LON.");
        return NAKresponse();
    }

    // Read the MSB of the number of bytes
    if(readByte(readBuffer) == 1){
        responseHeader[1] = readBuffer[0];
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read numBytes MSB character from LON.");
        return NAKresponse();
    }

    // Read the LSB of the number of bytes
    if(readByte(readBuffer) == 1){
        responseHeader[2] = readBuffer[0];
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read numBytes LSB character from LON.");
        return NAKresponse();
    }

    // Calculate response length based on header and allocate a response buffer
    responseLength = (responseHeader[1] << 8) + responseHeader[2];
    response = (unsigned char*)malloc(sizeof(unsigned char)*responseLength);

    // Copy Header
    for(i=0; i < 3; i++){
        response[i] = responseHeader[i];
    }

    // Read Command ID
    if(readByte(readBuffer) == 1){
        response[3] = readBuffer[0];
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read command ID character from LON.");
        free(response);
        return NAKresponse();
    }

    // Read Data
    for(i=4; i < responseLength - 1; i++){
        if(readByte(readBuffer) == 1){
            response[i] = readBuffer[0];
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to read data characteri %d of %d from LON.",i,responseLength);
            free(response);
            return NAKresponse();
        }
    }
    
    // Read Checksum
    if(readByte(readBuffer)){
        response[responseLength-1] = readBuffer[0];
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read checksum character from LON.");
        free(response);
        return NAKresponse();
    }

    return response;
}

LONresponse_s* createLONResponse(unsigned char* buffer){
    LONresponse_s* retVal = NULL;

    if(buffer){
        retVal = (LONresponse_s*)malloc(sizeof(LONresponse_s));
        if(buffer[0] == ACK || buffer[0] == NAK){
            retVal->deviceID = buffer[0];
            retVal->numBytes = 4;
            retVal->commandID = buffer[0];
            retVal->dataLength = 0;
            retVal->data = NULL;
            retVal->checkSum = (unsigned char)((buffer[0]*2)+4) & 0xFF;
            //syslog(LOG_DAEMON|LOG_ERR,"ACK or NAK command received.");
        } else {
            retVal->deviceID = buffer[0];
            retVal->numBytes = (buffer[1] << 8) + buffer[2];
            retVal->commandID = buffer[3];
            retVal->dataLength = retVal->numBytes - 5; // dataLength = numBytes - deviceIDByte - numByteMSB - numByteLSB - commandIDByte - checkSumByte
            //syslog(LOG_DAEMON|LOG_INFO,"LON Device ID: %02X, Num Bytes: %d, Command ID: %02X, Data Length: %d.",retVal->deviceID,retVal->numBytes,retVal->commandID,retVal->dataLength);
            retVal->data = (unsigned char*)malloc(sizeof(unsigned char)*retVal->dataLength);
            memcpy(retVal->data,buffer+4,retVal->dataLength);
            retVal->checkSum = buffer[retVal->numBytes-1];
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"No LON Response Received!");
    }
    return retVal;
}
/*
void logLONResponse(LONresponse_s* response){
    unsigned char* LONbytes;
    unsigned char dataByteTemp[4];
    unsigned int dataLength;
    unsigned int i;

    if(response){
        dataLength = response->numBytes - 5;
        LONbytes = malloc(sizeof(unsigned char)*(response->numBytes*4));
        LONbytes[0] = '\0';
        // Print the header
        sprintf(LONbytes,"%02X %02X %02X %02X",response->deviceID,(unsigned char)(response->numBytes&0xFF00)>>8,(unsigned char)(response->numBytes&0xFF),response->commandID);
        // Concatenate the data bytes
        for(i=0; i < dataLength; i++){
            sprintf(dataByteTemp,"%02X ",response->data[i]);
            strcat(LONbytes,dataByteTemp);
        }
        // Concatenate the checksum
        sprintf(dataByteTemp,"%02X ",response->checkSum);
        strcat(LONbytes,dataByteTemp);
        // Log the LONbytes string
        syslog(LOG_DAEMON|LOG_INFO,"LON Response: %s", LONbytes); 
    } else {
        syslog(LOG_DAEMON|LOG_INFO,"Trying to log null response.");
    }
}
*/
LONresponse_s* sendLONCommand(unsigned char device, unsigned char command, unsigned int dataLength, unsigned char* data){    

    // The length of a LON command is determined by:
    // [DEV ID] [TOT #BYTES MSB] [TOT #BYTES LSB] [COMMAND ID] [DATA] [BCC]
    unsigned int i,check,length = 1 + 1 + 1 + 1 + dataLength + 1;
    unsigned char* commBuffer = (unsigned char*)malloc(sizeof(unsigned char)*length);
    LONresponse_s* response = NULL;

    char hexChar[4];
    char debugLine[1025];
    debugLine[0] = '\0';

    if(portID == -1){
        // Kludgy, but this should be a rare case.
        free(commBuffer);
        commBuffer = NAKresponse();
        response = createLONResponse(commBuffer);
        free(commBuffer);
        return response; 
    }

    commBuffer[0] = device;
    commBuffer[1] = (unsigned char)(length >> 8) & 0xFF;
    commBuffer[2] = (unsigned char)(length & 0xFF);
    commBuffer[3] = command;
    for(i=0; i < dataLength; i++){
        commBuffer[i+4] = data[i];
    }

    // Calculate checksum
    check = 0;
    for(i=0; i < length-1; i++){
        check += commBuffer[i];
    }
    commBuffer[length-1] = (unsigned char)check & 0xFF;    
 
    pthread_mutex_lock(&LONportMutex);
    for(i=0; i < length && i < 341 ; i++){
        sprintf(hexChar,"%02X ",commBuffer[i]);
        strcat(debugLine,hexChar);
    }
//    syslog(LOG_DAEMON|LOG_INFO,"Sending LON %d bytes: %s.",length,debugLine);
    write(portID,commBuffer,length);
    tcdrain(portID);
    free(commBuffer);
    commBuffer = readLONResponse();
    pthread_mutex_unlock(&LONportMutex);

    usleep(200);

    response = createLONResponse(commBuffer);
    free(commBuffer);

    return response;
}

void freeLONResponse(LONresponse_s* response){
    if(response){
        if(response->data){
            free(response->data);
        }
        free(response);
    }
}


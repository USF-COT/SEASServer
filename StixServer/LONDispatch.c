#include "LONDispatch.h"

#define LONREADBUFFERSIZE 128

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
        sleep(1);
    }
    pthread_exit(NULL);
}

// clags and c_cc lines derived from: http://en.wikibooks.org/wiki/Serial_Programming/termios
int connect(char* port){
    int fd;
    struct termios settings;

    fd = open(port,O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd != -1){
        // Get default port settings
        if(tcgetattr(fd,&settings)==0){
            // Set the terminal to raw so that it does not process special characters
            cfmakeraw(&settings);
            // Set speed
            cfsetspeed(&settings,B19200);
            settings.c_cflag |= (CS8|CREAD|CLOCAL); // 8 bits, enable receiver, ignore modem control lines
            settings.c_cc[VMIN]=1;
            settings.c_cc[VTIME]=10;
            // Commit settings to open port
            if(tcsetattr(fd,TCSANOW,&settings) == 0){
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
        usleep(200);
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
    unsigned char* response = malloc(sizeof(unsigned char)*2);

    response[0] = NAK;
    response[1] = '\0';
    return response;
}

// Private method meant to clean up code a bit
unsigned char* readLONResponse(){
    int i,totalBytesRead,bytesRead,tries=0;
    unsigned int responseLength,dataLength;
    unsigned char* response;
    unsigned char responseHeader[3];

    const int MAX_TRIES = 3;

    response = calloc(LONREADBUFFERSIZE,sizeof(unsigned char));
    bytesRead = read(portID,response,LONREADBUFFERSIZE);
    if(bytesRead > 0){
        syslog(LOG_DAEMON|LOG_INFO,"Read %d bytes from LON.",bytesRead);
        return response;
    } else {
        syslog(LOG_DAEMON|LOG_INFO,"Unable to read from LON.  Returning NAK by default.");
        free(response);
        return NAKresponse();
    }
}

LONresponse_s* createLONResponse(unsigned char* buffer){
    LONresponse_s* retVal = NULL;
    unsigned int dataLength;

    if(buffer){
        retVal = malloc(sizeof(LONresponse_s));
        if(buffer[0] == ACK || buffer[0] == NAK){
            retVal->deviceID = buffer[0];
            retVal->numBytes = 4;
            retVal->commandID = buffer[0];
            retVal->data = NULL;
            retVal->checkSum = (unsigned char)((buffer[0]*2)+4) & 0xFF;
            syslog(LOG_DAEMON|LOG_ERR,"ACK command received.");
        } else {
            retVal->deviceID = buffer[0];
            retVal->numBytes = (buffer[1] << 8) + buffer[2];
            retVal->commandID = buffer[3];
            dataLength = retVal->numBytes - 5; // dataLength = numBytes - deviceIDByte - numByteMSB - numByteLSB - commandIDByte - checkSumByte
            syslog(LOG_DAEMON|LOG_INFO,"LON Device ID: %02X, Num Bytes: %d, Command ID: %02X, Data Length: %d.",retVal->deviceID,retVal->numBytes,retVal->commandID);
            retVal->data = malloc(sizeof(unsigned char)*dataLength);
            memcpy(retVal->data,buffer+4,dataLength);
            retVal->checkSum = buffer[dataLength+4];
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
    unsigned char* commBuffer = malloc(sizeof(unsigned char)*length);
    unsigned char responseHeader[3];
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
    syslog(LOG_DAEMON|LOG_INFO,"Sending LON %d bytes: %s.",length,debugLine);
    write(portID,commBuffer,length);
    tcdrain(portID);
    free(commBuffer);
    sleep(1);
    commBuffer = readLONResponse();
    pthread_mutex_unlock(&LONportMutex);

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


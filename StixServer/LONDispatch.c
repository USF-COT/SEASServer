#include "LONDispatch.h"

int portID;
volatile sig_atomic_t dispatching = 0;
pthread_mutex_t LONportMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t LONdispatchThread;

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

int connect(char* port){
    int fd;
    struct termios settings;

    fd = open(port,O_ASYNC);
    if(fd != -1){
        // Get default port settings
        if(tcgetattr(fd,&settings)==0){
            // Set speed
            cfsetispeed(&settings,B19200);
            cfsetospeed(&settings,B19200);
            // Set 8 Data Bits
            settings.c_cflag |= CS8;
            // Ignore Parity (No Parity)
            settings.c_iflag |= IGNPAR;
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
    unsigned char readBuffer[3];

    const int MAX_TRIES = 3;

    // Read Device ID
    if(readByte(readBuffer) == 1){
        if(readBuffer[0] == ACK){
            response = malloc(sizeof(unsigned char)*2);
            response[0] = ACK;
            response[1] = '\0';
            return response;
        } else {
            responseHeader[0] = readBuffer[0];
        }
    } else { // Unable to read the device character from the LON Port, return error code NAK
        return NAKresponse();
    }

    // Read the MSB of the number of bytes
    if(readByte(readBuffer) == 1){
        responseHeader[1] = readBuffer[0];
    } else {
        return NAKresponse();
    }

    // Read the LSB of the number of bytes
    if(readByte(readBuffer) == 1){
        responseHeader[2] = readBuffer[0];
    } else {
        return NAKresponse();
    }

    // Calculate the response length and allocate response string
    // Response Length = Header(3 Bytes) + Command ID + Data Length + Checksum + '\0' at the end for good measure
    dataLength = (responseHeader[1] << 8) + responseHeader[2];
    responseLength = 3 + 1 + dataLength + 1 + 1;
    response = malloc(sizeof(unsigned char)*responseLength);

    // Copy Header
    for(i=0; i < 3; i++){
        response[i] = responseHeader[i];
    }

    // Read Command ID
    if(readByte(readBuffer) == 1){
        response[4] = readBuffer[0];
    } else {
        free(response);
        return NAKresponse();
    }

    // Read Data
    for(i=5; i < responseLength - 2; i++){
        if(readByte(readBuffer) == 1){
            response[i] = readBuffer[0];
        } else {
            free(response);
            return NAKresponse();
        }
    }
    
    // Read Checksum
    if(readByte(readBuffer)){
        response[responseLength-2] = readBuffer[0];
    } else {
        free(response);
        return NAKresponse();
    }

    // Add '\0' at the end for good measure
    response[responseLength-1] = '\0';
    return response;
}

unsigned char* sendLONCommand(unsigned char device, unsigned char command, unsigned int dataLength, unsigned char* data){
    
    // The length of a LON command is determined by:
    // [DEV ID] [TOT #BYTES MSB] [TOT #BYTES LSB] [COMMAND ID] [DATA] [BCC]
    unsigned int i,check,length = 1 + 1 + 1 + 1 + dataLength + 1;
    unsigned char* commBuffer = malloc(sizeof(unsigned char)*length);
    unsigned char responseHeader[3];

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
    write(portID,commBuffer,length);
    free(commBuffer);
    commBuffer = readLONResponse();
    pthread_mutex_unlock(&LONportMutex);

    return commBuffer;
}


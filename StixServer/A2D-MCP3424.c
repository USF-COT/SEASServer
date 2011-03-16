#include "A2D-MCP3424.h"

#define MCP3424PORT "/dev/i2c-0"
#define MCP3424ADDR 0x68

static a2dMCP3424* a2d = NULL;
static pthread_mutex_t a2dMutex = PTHREAD_MUTEX_INITIALIZER;

a2dMCP3424* createMCP3424(const char* port,uint8_t addressBits){
    a2dMCP3424* retVal = malloc(sizeof(struct A2DMCP3424));

    // Copy Port Name and Address Bits
    retVal->port = malloc(sizeof(char)*(strlen(port)+1));
    strncpy(retVal->port,port,strlen(port)+1);
    retVal->addressBits = addressBits;

    // Set Default Parameters for MCP3424
    setConfiguration(0,0,ONESHOT,SPS3_75,PGAX1);
    return retVal;
}

void destroyMCP3424(a2dMCP3424* handle){
    free(handle->port);
    free(handle);
}

void initA2D(){
    pthread_mutex_lock(&a2dMutex);
    if(a2d == NULL){
        a2d = createMCP3424(MCP3424PORT,MCP3424ADDR);
    }
}

void freeA2D(){
    if(a2d != NULL){
        destroyMCP3424(a2d);
    }
    pthread_mutex_unlock(&a2dMutex);
}

BOOL setConfiguration(BOOL RDY, uint8_t channel, MCP3424ConvMode mode, MCP3424SampRate rate, MCP3424PGAGain gain){
    int fh;
    uint8_t writeByte = 0;
    uint8_t baseAddress = 0x68;

    initA2D();
    
    fh = open(a2d->port,O_RDWR);
    if(fh){
        baseAddress |= (a2d->addressBits & 0x07);
        ioctl(fh,I2C_SLAVE,baseAddress);
        writeByte |= (RDY << 7) | ((channel & 0x03) << 5) | ((mode & 0x01) << 4) | ((rate & 0x03) << 2) | (gain & 0x03);
        if((write(fh,&writeByte,1) > 0)){
            a2d->channel = channel;
            a2d->mode = mode;
            a2d->rate = rate;
            a2d->gain = gain;
            close(fh);
            freeA2D();
            return TRUE;
        }
        close(fh);
    } 
    freeA2D();
    return FALSE;
}

int readChannel(){
    int fh,retVal = -1;
    uint8_t baseAddress = 0;
    uint8_t readBytes[5];
 
    initA2D();

    fh = open(a2d->port,O_RDWR);
    if(fh){
        baseAddress |= (a2d->addressBits & 0x07); 
        ioctl(fh,I2C_SLAVE,baseAddress);
        if(read(fh,readBytes,5) == 5){
            retVal = 0;
            switch(a2d->mode){
                case SPS3_75:
                   retVal |= ((readBytes[1] & 0x03) << 13) | (readBytes[2] << 8) | readBytes[3];
                   break;
                case SPS15:
                    retVal |= (readBytes[1] << 8) | readBytes[2];
                    break;
                case SPS60:
                    retVal |= ((readBytes[1] & 0x3F) << 8) | readBytes[2]; 
                    break;
                case SPS240:
                    retVal |= ((readBytes[1] & 0x1F) << 8) | readBytes[2];
                    break;
            }
        }                
        close(fh);
    } 
    freeA2D();
    return retVal;
}


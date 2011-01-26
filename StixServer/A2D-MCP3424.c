#include "A2D-MCP3424.h"

a2dMCP3424* createMCP3424(const char* port,uint8_t addressBits){
    a2dMCP3424* retVal = malloc(sizeof(struct A2DMCP3424));

    // Copy Port Name and Address Bits
    retVal->port = malloc(sizeof(char)*(strlen(port)+1));
    strncpy(retVal->port,port,strlen(port)+1);
    retVal->addressBits = addressBits;

    // Set Default Parameters for MCP3424
    setConfiguration(retVal,0,0,ONESHOT,SPS3_75,PGAX1);
    return retVal;
}

void destroyMCP3424(a2dMCP3424* handle){
    free(handle->port);
    free(handle);
}

BOOL setConfiguration(a2dMCP3424* handle, BOOL RDY, uint8_t channel, MCP3424ConvMode mode, MCP3424SampRate rate, MCP3424PGAGain gain){
    int fh;
    uint8_t writeByte = 0;
    uint8_t baseAddress = 0x68;

    fh = open(handle->port,O_RDWR);
    if(fh){
        baseAddress |= (handle->addressBits & 0x07);
        ioctl(fh,I2C_SLAVE,baseAddress);
        writeByte |= (RDY << 7) | ((channel & 0x03) << 5) | ((mode & 0x01) << 4) | ((rate & 0x03) << 2) | (gain & 0x03);
        if((write(fh,&writeByte,1) > 0)){
            handle->channel = channel;
            handle->mode = mode;
            handle->rate = rate;
            handle->gain = gain;
            close(fh);
            return TRUE;
        }
        close(fh);
    } 
    return FALSE;
}

// Reads in all channels up to maxChannel then returns them as an array terminated by a NULL pointer element.
int readChannel(a2dMCP3424* handle,uint8_t channel){
    int fh,retVal = -1;
    uint8_t baseAddress = 0;
    uint8_t readBytes[5];
 
    setConfiguration(handle,1,channel,handle->mode,handle->rate,handle->gain);
    milliSleep(300); 

    fh = open(handle->port,O_RDWR);
    if(fh){
        baseAddress |= (handle->addressBits & 0x07); 
        ioctl(fh,I2C_SLAVE,baseAddress);
        if(read(fh,readBytes,5) == 5){
            retVal = 0;
            switch(handle->mode){
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
    return retVal;
}


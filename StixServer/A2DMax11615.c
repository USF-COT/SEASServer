#include "A2DMax11615.h"

a2dMAX11615* connectMAX11615(const char* port){
    int fh = -1;
    a2dMAX11615* a2d = NULL;

    fh = open(port,O_RDWR);
    if(fh){
        ioctl(fh,I2C_SLAVE,0x33);
        a2d = malloc(sizeof(a2dMAX11615));
        a2d->connected = TRUE;
        a2d->port = fh;
        a2d->maxChannel = 1;
        return a2d;
    } else {
        return NULL;
    }
}

int setSetupParams(a2dMAX11615* a2d, BOOL internRef,BOOL refOut, BOOL intRefSt, BOOL extClock, BOOL bipolar){
    unsigned char setupByte = 0x80;    

    if(a2d && a2d->connected){
        setupByte |= internRef ? 0x40 : 0;
        setupByte |= refOut ? 0x20 : 0;
        setupByte |= intRefSt ? 0x10 : 0;
        setupByte |= extClock ? 0x08 : 0;
        setupByte |= bipolar ? 0x04 : 0;
        setupByte |= 0x02; 
        if(write(a2d->port,&setupByte,1) == 1){
            a2d->internRef = internRef;
            a2d->refOut = refOut;
            a2d->intRefSt = intRefSt;
            a2d->extClock = extClock;
            a2d->bipolar = bipolar;
            return TRUE;
         } else {
             return FALSE;
         }
    } else {
        return FALSE;
    }
}
int setConfigParams(a2dMAX11615* a2d, scanType scan, unsigned short maxChannel, BOOL singleEnded){
    unsigned char configByte = 0x00;

    if(a2d && a2d->connected){
        configByte |= (scan & 0x03) << 6;
        configByte |= (maxChannel & 0x0F) << 1;
        configByte |= singleEnded ? 0x01 : 0;
        if(write(a2d->port,&configByte,1) == 1){
            a2d->scan = scan;
            a2d->maxChannel = maxChannel;
            a2d->singleEnded = singleEnded;
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

// Reads in all channels up to max channel
int* readChannels(a2dMAX11615* a2d){
    unsigned int i;
    unsigned int numBytes = 0;
    unsigned char* rawArray = NULL;
    int* retArray = NULL;

    if(a2d && a2d->connected){
        numBytes = 3*a2d->maxChannel;
        rawArray = malloc(numBytes);
        if(read(a2d->port,rawArray,numBytes) == numBytes){
           retArray = malloc(sizeof(int) * (a2d->maxChannel+1));
           for(i=0; i < numBytes; i+3){
               retArray[i/3] = ((int)rawArray[i] << 8) | ((int)rawArray[i+1] << 4) | (int)rawArray[i+2];
           } 
           free(rawArray);
        }
    } 
    return retArray;
}
void disconnectMAX11615(a2dMAX11615* a2d){
    close(a2d->port);
}

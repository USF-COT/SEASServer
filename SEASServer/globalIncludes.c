#include "globalIncludes.h"

int milliSleep(unsigned int millisecondsToSleep){
    usleep(millisecondsToSleep);
    return 1;
}

char* byteArrayToString(unsigned char* bytes, unsigned int length){
    int i;
    char byteValue[5];
    char* string = calloc((length*3)+1, sizeof(char));
    string[0] = '\0';
    
    for(i=0; i < length; i++){
        byteValue[0] = '\0';
        sprintf(byteValue,"%02X ",bytes[i]);
        strcat(string,byteValue);
    }

    return string;
}

void copyReverseBytes(void* dest, const void* src, unsigned int numBytes){
    int i;
    unsigned char* destBytes = (unsigned char*)dest;
    unsigned char* bytes = calloc(numBytes,sizeof(unsigned char));
    memcpy(bytes,src,numBytes);

    if(destBytes){
        for(i=0; i < numBytes; i++){
            destBytes[i] = bytes[(numBytes-1)-i];
        }
    }
    free(bytes);
}

void soundBeep(){
    return;
}



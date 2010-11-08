#include "globalIncludes.h"

int milliSleep(unsigned int millisecondsToSleep){
    int usleepDev;
    int retVal = 1;
    const char* usleepDevName = "/dev/usleep";

    if((usleepDev = open(usleepDevName,O_RDWR)) < 0){
        return -1;
    }

    if((ioctl(usleepDev, USLEEP_IOCTL_MDELAY, millisecondsToSleep)) != 0){
        retVal = -1;
    }
    close(usleepDev);
    return retVal;
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

// Taken from: http://www.developer.com/open/article.php/631191
void soundBeep(){
    char beep[2] = {7,'\0'};
    printf("%c",beep);
}

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

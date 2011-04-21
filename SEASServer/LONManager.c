/* LONManager.c - Defines communications with the LON nodes and stores data retrieved from the LON network.
 *
 * By: Michael Lindemuth
 */

#include <stdio.h>
#include <stdlib.h>
#include "LONManager.h"

#define PORT "/dev/ttyS0"

static int fd;

int connectLON(){
    fd = open(PORT,O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd != -1)
    
}

void disconnectLON(){
    fd = 
}

void sendData(unsigned char* data){

}

unsigned char* retrieveData(unsigned int numBytes){

}

CTD_SENSOR_DATA* getCTDSensorData(){
    CTD_SENSOR_DATA* data = malloc(sizeof(CTD_SENSOR_DATA));
    
    connectLON();     
}



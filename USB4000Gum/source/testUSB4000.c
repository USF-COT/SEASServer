/*
* USB4000 Test Application
* 
* testUSB4000.c - Opens, collects data from, and closes connection to USB4000 using only the USB4000Gum driver.
*
*/

#define DEBUG 1

#include <stdio.h>
#include "USB4000Gum.h"

#define SPECID 1

int main(){

    int i;
    spectrometer* spectrometers[2];
    specSample* sample;
    char* response;

    printf("Testing USB4000 Driver Library\n"); 
    
    printf("Opening Device\n");
    spectrometers[0] = openUSB4000("USB4F02570");
    spectrometers[1] = openUSB4000("USB4F02572");

    if(spectrometers[SPECID] != NULL){
        printf("Opened.\n");

        setIntegrationTime(spectrometers[SPECID],10);

        // Test Query Config
        response = queryConfig(spectrometers[SPECID],2);

        printf("Config Response: ");
        for(i=2; i < 18; i++)
            printf("%c",response[i]);
        printf("\n");

        // Trigger Mode
        setTriggerMode(spectrometers[SPECID],NORMAL);
        
        printf("Getting Sample Over 10 Scans.\n");
        sample = getSample(spectrometers[SPECID],10,100);
        printf("Got'em!\n");
        

        printf("Spectrometer Status\n");
        printf("-------------------\n");
        printStatus(spectrometers[SPECID]);
        printf("-------------------\n");

        printf("Closing Device\n");
        closeUSB4000(spectrometers[SPECID]);
        printf("Closed.\n");
    }
    else{
        printf("Unable to connect to device.\n");
    }

    
    return 0;
}

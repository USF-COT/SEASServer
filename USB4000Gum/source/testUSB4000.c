/*
* USB4000 Test Application
* 
* testUSB4000.c - Opens, collects data from, and closes connection to USB4000 using only the USB4000Gum driver.
*
*/

#define DEBUG 1

#include <stdio.h>
#include "USB4000Gum.h"

int main(){

    int i;
    spectrometer *USB4000 = NULL;
    specSample* sample;
    char* response;

    printf("Testing USB4000 Driver Library\n"); 
    
    printf("Opening Device\n");
    USB4000 = openUSB4000("USB4F02572");

    if(USB4000 != NULL){
        printf("Opened.\n");

        printf("Initializing Device\n");
        initDevice(USB4000);
        printf("Initialized\n");

        setIntegrationTime(USB4000,7000);

        // Test Query Config
        response = queryConfig(USB4000,2);

        printf("Config Response: ");
        for(i=2; i < 18; i++)
            printf("%c",response[i]);
        printf("\n");

        // Trigger Mode
        setTriggerMode(USB4000,NORMAL);
        
        printf("Getting Sample Over 10 Scans.\n");
        sample = getSample(USB4000,10,50);
        printf("Got'em!\n");
        

        printf("Spectrometer Status\n");
        printf("-------------------\n");
        printStatus(USB4000);
        printf("-------------------\n");

        printf("Closing Device\n");
        closeUSB4000(USB4000);
        printf("Closed.\n");
    }
    else{
        printf("Unable to connect to device.\n");
    }

    
    return 0;
}

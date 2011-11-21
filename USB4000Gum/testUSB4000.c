/*
* USB4000 Test Application
* 
* testUSB4000.c - Opens, collects data from, and closes connection to USB4000 using only the USB4000Gum driver.
*
*/

#define DEBUG 1

#include <stdio.h>
#include <unistd.h>
#include "libUSB4000.h"

#define NUM_SPECS 2

int main(){

    int i;
    int specID;
    spectrometer* spectrometers[2];
    specSample* sample;
    char* response;

    printf("Testing USB4000 Driver Library\n"); 
    
    printf("Opening Device\n");
    spectrometers[0] = openUSB4000("USB4F02570");
    spectrometers[1] = openUSB4000("USB4F02571");

    for(specID=0; specID < NUM_SPECS; specID++){
        printf("Opened.\n");

        setIntegrationTime(spectrometers[specID],10);

        // Test Query Config
        response = queryConfig(spectrometers[specID],2);

        printf("Config Response: ");
        for(i=2; i < 18; i++)
            printf("%c",response[i]);
        printf("\n");

        // Trigger Mode
        setTriggerMode(spectrometers[specID],NORMAL);
        
        usleep(4000);
        
        printf("Getting Sample.\n");
        sample = getSample(spectrometers[specID],10,100,5);
        printf("Got'em!\n");
        

        printf("Spectrometer Status\n");
        printf("-------------------\n");
        printStatus(spectrometers[specID]);
        printf("-------------------\n");

        printf("Shutdown Spectrometers\n");
        printf("-------------------\n");
        setShutdownMode(spectrometers[0],FALSE);
        setShutdownMode(spectrometers[1],FALSE);
        printf("-------------------\n");

        printf("Press Enter to Power Up Spectrometers.\n");
        while(getchar() != '\n'){};

        printf("Starting Spectrometers\n");
        printf("-------------------\n");
        setShutdownMode(spectrometers[0],TRUE);
        setShutdownMode(spectrometers[1],TRUE);
        printf("-------------------\n");

    }

    printf("Closing Device\n");
    closeUSB4000(spectrometers[0]);
    closeUSB4000(spectrometers[1]);
    printf("Closed.\n");
    
    return 0;
}

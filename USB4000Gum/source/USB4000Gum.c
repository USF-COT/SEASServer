/*
* Ocean Optics USB4000 Small Driver for Gumstix
* 
* USB4000Gum.c - A scaled down version of the OmniDriver build by Ocean Optics.
*                Provides connection to the device, and data collection functions.
*/

#include "USB4000Gum.h"

#define DEBUG 1

spectrometer* allocateSpec(){
    spectrometer* USB4000 = malloc(sizeof(spectrometer));
    USB4000->status = malloc(sizeof(specStatus));
    USB4000->calibration = malloc(sizeof(calibrationCoefficients));
    USB4000->sample = NULL;
    USB4000->serialNumber = NULL;

    return USB4000;
}

void deallocateSpec(spectrometer* USB4000){
    if(USB4000->sample != NULL)
        free(USB4000->sample);

    free(USB4000->calibration);
    free(USB4000->status);

    if(USB4000->serialNumber != NULL)
        free(USB4000->serialNumber);

    free(USB4000);
}

// Device Connectivity
spectrometer* openUSB4000(const char* serialNumber){
    int claimRetVal,interface;
    char* devNumber;
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *usbHandle;
    spectrometer* USB4000 = NULL;

    claimRetVal = 0;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for(bus = usb_get_busses(); bus; bus = bus->next){
        for(dev = bus->devices; dev; dev = dev->next){

            if((dev->descriptor.idVendor == 0x2457) && (dev->descriptor.idProduct == 0x1022)){
                usbHandle = usb_open(dev);
                interface = dev->config->interface->altsetting->bInterfaceNumber;
                
                if(usbHandle != NULL){ // If the handle is successfully open
                    claimRetVal = usb_claim_interface(usbHandle,interface);

                    if(claimRetVal == 0){
                        //Allocate Spectrometer
                        USB4000 = allocateSpec();
                        USB4000->usbHandle = usbHandle;
                        USB4000->interface = interface;

                        initDevice(USB4000);
                        // Check if the found device matches the requested serial number
                        devNumber = queryConfig(USB4000,0);
                        if(strcmp(devNumber+2,serialNumber) == 0){ // Device found!
                            // Store USB4000 Serial Number in Struct
                            USB4000->serialNumber = malloc(strlen(serialNumber)*sizeof(char));
                            strncpy(USB4000->serialNumber,serialNumber,strlen(serialNumber));

                            #ifdef DEBUG
                                fprintf(stderr,"Found and Opened USB4000 with serial number: %s\n",devNumber+2);
                            #endif

                            free(devNumber);
                            return USB4000;
                        }
                        else{
                            #ifdef DEBUG
                                fprintf(stderr,"Found USB4000 with serial number: %s.  Looking for: %s.\n",devNumber+2,serialNumber);
                            #endif
                            usb_release_interface(USB4000->usbHandle,USB4000->interface);
                            usb_close(USB4000->usbHandle);
                            deallocateSpec(USB4000);
                            #ifdef DEBUG
                                fprintf(stderr,"USB4000 Spectrometer Deallocated.");
                            #endif
                            free(devNumber);
                        }

                    }
                    else{
                        #ifdef DEBUG
                            fprintf(stderr,"%s\n",usb_strerror());
                            fprintf(stderr,"Cannot claim USB4000.  Looking for other devices...\n");
                        #endif
                        usb_close(usbHandle);
                    }
                }
                else{
                    #ifdef DEBUG
                        fprintf(stderr, "Cannot open USB4000 device.  Trying others...\n");
                    #endif
                }
            }
        }
    }

    #ifdef DEBUG
        fprintf(stderr,"FAILED: Unable to Open a Connection to USB4000.\n");
    #endif
    return NULL;
}

STATUS closeUSB4000(spectrometer* USB4000){
    if(isConnected(USB4000)){

        // Release USB Resources
        usb_release_interface(USB4000->usbHandle,USB4000->interface);
        usb_close(USB4000->usbHandle);

        // Free Up Memory
        deallocateSpec(USB4000);

        #ifdef DEBUG
            fprintf(stderr,"Opened USB4000 Closed.\n");
        #endif

        return USB4000OK;
    }
    else{
        #ifdef DEBUG
            fprintf(stderr,"Unopened USB4000 Cannot Be Closed.\n");
        #endif

        return USB4000ERR;
    }
}

// Convenience Methods
BOOL isConnected(spectrometer* USB4000){
    return USB4000->usbHandle != NULL;
}

unsigned int UnsignedLEtoInt(unsigned char* LittleEndianString, unsigned short numChars){
    int i;
    unsigned int retVal = 0;

    for(i=0; i < numChars; i++){
        retVal |= (unsigned int)(LittleEndianString[i] << (8*i));
    }

    return retVal;
}

STATUS updateStatus(spectrometer* USB4000){
    int numBytesWritten,numBytesRead;
    char command[1];
    unsigned char response[16];

    numBytesWritten = numBytesRead = 0;

    if(isConnected(USB4000)){
        command[0] = 0xFE;
        numBytesWritten = usb_bulk_write(USB4000->usbHandle,EP1OUT,command,1,1000);
        numBytesRead = usb_bulk_read(USB4000->usbHandle,EP1IN,response,16,1000);

        // Parse Response
        USB4000->status->numPixels = (unsigned short)UnsignedLEtoInt(response,2);
        USB4000->status->integrationTime = (unsigned int) UnsignedLEtoInt(response+2,4);
        USB4000->status->lampStatus = (response[6] != 0) ? TRUE : FALSE;
        USB4000->status->triggerMode = (TRIGGER) response[7];
        USB4000->status->spectralAcquisitionStatus = (response[8] == 1) ? TRUE : FALSE;
        USB4000->status->numSpectraPackets = response[9];
        USB4000->status->isOn = (response[10] != 0) ? TRUE : FALSE;
        USB4000->status->numPacketsInEnd = response[11];
        USB4000->status->isHighSpeed = ((response[14]) == 0x80) ? TRUE : FALSE;
    }

    return USB4000OK;
}

void printStatus(spectrometer* USB4000){
    printf("Serial Number: %s\n", USB4000->serialNumber);
    printf("Number of Pixels: %i\n",USB4000->status->numPixels);
    printf("Integration Time: %i\n", USB4000->status->integrationTime);
    printf("Lamp Status: %s\n", USB4000->status->lampStatus ? "On" : "Off");
    printf("Trigger Mode: ");
    switch(USB4000->status->triggerMode){
        case NORMAL:
            printf("%s\n","NORMAL");
            break;
        case SOFTWARE:
            printf("%s\n","SOFTWARE");
            break;
        case EXSYNC:
            printf("%s\n", "EXSYNC");
            break;
        case EXHARD:
            printf("%s\n", "EXHARD");
            break;
        case QREAL:
            printf("%s\n", "QREAL");
            break;
        default:
            printf("UNKNOWN\n");
    }
    printf("Spectral Acquisition Status: %i\n",USB4000->status->spectralAcquisitionStatus);
    printf("Number of Spectral Packets: %i\n", USB4000->status->numSpectraPackets);
    printf("Unit On: %s\n", USB4000->status->isOn ? "Off" : "On");
    printf("Number End Point Packets: %i\n", USB4000->status->numPacketsInEnd);
    printf("USB: %s\n", USB4000->status->isHighSpeed ? "High Speed" : "Full Speed (Low Speed)");
}

// Command Methods
STATUS initDevice(spectrometer* USB4000){
    char command[2];
    char response[17];

    // Send Initialize Command
    command[0] = 0x01;
    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,1,1000);

    // Get Saturation Level
    command[0] = 0x05;
    command[1] = 0x11;
    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,2,1000);
    usb_bulk_read(USB4000->usbHandle,EP1IN,response,17,1000);

    USB4000->saturation_level = ((response[7] & 0x00FF) << 8) | response[6];

    updateWavelengthCalibrationCoefficients(USB4000);
    updateStatus(USB4000);

    return USB4000OK;
}

void swapBytes(char* bytes, unsigned int numBytes){
    unsigned int i,j;
    char temp;

    for(i=0,j=numBytes-1; i < numBytes && i != j; i++,j--){
        temp = bytes[i];
        bytes[i] = bytes[j];
        bytes[j] = temp; 
    }

}

STATUS setIntegrationTime(spectrometer* USB4000,unsigned int time){
    char command[5];
    int i;

    if(USB4000 == NULL)
    {
        fprintf(stderr,"Invalid Spectrometer Pointer Passed.");
        return USB4000ERR;
    }

    command[0] = 0x02;
    memcpy(command+1,&time,4);
    swapBytes(command+1,4);

#ifdef DEBUG
    fprintf(stderr,"Integration Time %i Command:",time);
    for(i = 0; i < 5; i++)
    {
        fprintf(stderr," 0x%02X",command[i]);
    }
    fprintf(stderr,"\n");
#endif    

    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,5,1000);

    updateStatus(USB4000);

    getSample(USB4000,1,100);

    return USB4000OK;
}

STATUS setStrobe(spectrometer* USB4000, BOOL on){
    char command[3];

    command[0] = 0x03;
    if(on)
        command[1] = 0x01;
    else
        command[1] = 0x00;
    command[2] = 0x00;

    usb_bulk_write(USB4000->usbHandle,EP1OUT, command, 3, 1000);

    updateStatus(USB4000);
    return USB4000OK;
}

STATUS setShutdownMode(spectrometer* USB4000, BOOL powerAll){
    char command[3];

    command[0] = 0x04;
    if(powerAll)
        command[1] = 0x01;
    else
        command[1] = 0x00;
    command[2] = 0x00;

    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,3,1000);

    updateStatus(USB4000);
    return USB4000OK;
}

char* queryConfig(spectrometer* USB4000, char paramNumber){
    char command[2];
    char* response;

    response = malloc(sizeof(char)*18);

    command[0] = 0x05;
    command[1] = paramNumber;

    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,2,1000);
    usb_bulk_read(USB4000->usbHandle,EP1IN,response,18,1000);

    return response;
}

STATUS writeConfig(spectrometer* USB4000, char paramNumber, char* writeASCII){
    char command[18];

    command[0] = 0x06;
    memcpy(command+1,writeASCII,17);

    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,2,1000);

    return USB4000OK;
}

STATUS setTriggerMode(spectrometer* USB4000, TRIGGER triggerMode){
    char command[3];

    command[0] = 0x0A;
    command[1] = triggerMode;
    command[2] = 0x00;

    usb_bulk_write(USB4000->usbHandle,EP1OUT,command,3,1000);

    updateStatus(USB4000);

    // Reset Array
    getSample(USB4000,1,100);
    return USB4000OK;
}

STATUS updateWavelengthCalibrationCoefficients(spectrometer* USB4000){
    int i;
    char* response;

    // Wavelength Order Coefficients
    for(i=1; i < 5; ++i){
        response = queryConfig(USB4000,i);
        USB4000->calibration->wavelengthOrder[i-1] = atof(response+2);
        free(response);
    }

    // Stray Light Coefficients
    response = queryConfig(USB4000,5);
    USB4000->calibration->strayLightConstant = atof(response+2);
    free(response);

    // Non-linear correction coefficients
    for(i=6; i < 14; ++i){
        response = queryConfig(USB4000,i);
        USB4000->calibration->nonLinearCorrectionOrder[i-6] = atof(response+2);
        free(response);
    }

    // Polynomial Order of Non-linearity correction
    response = queryConfig(USB4000,14);
    USB4000->calibration->polyOrderNonLinearCal = atof(response+2);
    free(response);

    return USB4000OK;
}

specSample* allocateSample(unsigned int numScansPerSample, unsigned short numPixels){
    specSample* sample;
    sample = malloc(sizeof(specSample));
    sample->numScansForSample = numScansPerSample;
    sample->pixels = calloc(numPixels,sizeof(float));

    return sample;
}

void deallocateSample(specSample* sample){
    free(sample->pixels);
    free(sample);
}

specSample* getSample(spectrometer* USB4000, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds){ // 0x09
    specSample* sample;
    char command[1];
    char response[7681];
    int i,j,k,numRead;
    short newPixel;
    float pixel;
    const float satScale = 65535.0/(float)USB4000->saturation_level;

    // Setup variables
    numRead = 0;
    if(USB4000->sample != NULL){
        deallocateSample(USB4000->sample);
        USB4000->sample = NULL;
    }
    sample = allocateSample(numScansPerSample, USB4000->status->numPixels);
    command[0] = 0x09;

    for(i=0; i < numScansPerSample; i++){
        if(usb_bulk_write(USB4000->usbHandle,EP1OUT,command,1,1000) > 0){

            // Wait for Pixels to Fill
            usleep(4000);
            // Handle Response Depending on USB Connection
            if(USB4000->status->isHighSpeed){
                fprintf(stderr,"Handling High-Speed USB Connection.");
                numRead = usb_bulk_read(USB4000->usbHandle, EP6IN,response,2048,1000);
                numRead += usb_bulk_read(USB4000->usbHandle, EP2IN,response+2048,5633,1000);
            }
            else{
                fprintf(stderr,"Handling Full Speed (Slower Speed) USB Connection.");
                numRead = usb_bulk_read(USB4000->usbHandle, EP2IN,response,7681,10000);
            }

            fprintf(stderr,"Read %d Bytes.  Sync byte (0x69) is 0x%x. Any errors?: %s\n",numRead,response[7680],usb_strerror());

            if((numRead == USB4000->status->numPixels*2 + 1) && response[7680] == 0x69){
                #ifdef DEBUG
                    fprintf(stderr,"Sync byte for sample %i correct!\n",i+1);
                #endif

                // Convert Each Pixel to Short
                for(j=0; j < USB4000->status->numPixels; j++){
                    newPixel = (unsigned short)UnsignedLEtoInt(response+(2*j),2);
                    pixel = ((float)newPixel) * satScale; 
                    sample->pixels[j] = (float)((sample->pixels[j]*i + pixel))/(float)(i+1);
                   fprintf(stdout,"%i,%i,%f,%f\n",j,newPixel,pixel,sample->pixels[j]);
                }
            }
            else{
                #ifdef DEBUG
                    fprintf(stderr, "Unable to read response.\n");
                #endif
               deallocateSample(sample);
               return NULL;
            }
            usleep(delayBetweenScansInMicroSeconds);
        }
        else{
            #ifdef DEBUG
                fprintf(stderr, "Unable to send command to USB4000.  Check connection.\n");
            #endif
            deallocateSample(sample);
            return NULL;
        }
    }

    USB4000->sample = sample;
    return sample;
}



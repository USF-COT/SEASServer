/*
* Ocean Optics USB4000 Small Driver for Gumstix
* 
* USB4000Gum.h - A scaled down version of the OmniDriver build by Ocean Optics.
*                Provides connection to the device, and data collection functions.
*/

#ifndef _USB4000GUM_H
#define _USB4000GUM_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <usb.h>
#include <math.h>
 
#define STATUS char
#define USB4000ERR 0
#define USB4000OK 1

#define BOOL char
#define TRUE 1
#define FALSE 0

#define NUMPIXELS 3840
#define BYTESPERPIXEL 2

/* Number of pixels used to compute the dark value */
#define START_DARK 5
#define MAX_DARK_PIXEL 18

/* Non-linear coefficient count */
#define  NONLINEAR_COEFFICIENT_COUNT         8

// End Point Definitions
#define EP1IN 0x81
#define EP1OUT 0x01
#define EP2IN 0x82
#define EP6IN 0x86

typedef enum {NORMAL,SOFTWARE,EXSYNC,EXHARD,QREAL}TRIGGER;

typedef struct SPECSTATUS{
    unsigned short numPixels;
    unsigned int integrationTime;
    BOOL lampStatus;
    TRIGGER triggerMode;
    BOOL spectralAcquisitionStatus;
    unsigned short numSpectraPackets;
    BOOL isOn;
    unsigned short numPacketsInEnd;
    BOOL isHighSpeed;
}specStatus;

typedef struct CALIBRATIONCOEFFICIENTS{
    float wavelengthOrder[4];
    float strayLightConstant;
    float nonLinearCorrectionOrder[8];
    unsigned short polyOrderNonLinearCal;
    unsigned short dummyShort;
}calibrationCoefficients;

typedef struct SPECSSAMPLE{
    unsigned short numScansForSample;
    float* pixels;
} specSample;

typedef struct SPECTROMETER{
    char* serialNumber;
    usb_dev_handle *usbHandle;
    int interface;
    specStatus* status;
    calibrationCoefficients* calibration;
    specSample* darkSample;
    specSample* refSample;
    specSample* sample;
    double* lambdaValues;
    int saturation_level;
}spectrometer;

// Device Connectivity
extern spectrometer* openUSB4000(const char* serialNumber);
extern BOOL isConnected(spectrometer* USB4000);
extern STATUS closeUSB4000(spectrometer* USB4000);
extern void deallocateSample(specSample** sample);

// USB Commands
extern STATUS initDevice(spectrometer* USB4000); // 0x01
extern STATUS setIntegrationTime(spectrometer* USB4000,unsigned int time); // 0x02
extern STATUS setStrobe(spectrometer* USB4000, BOOL on); // 0x03
extern STATUS setShutdownMode(spectrometer* USB4000, BOOL powerAll); // 0x04
extern char* queryConfig(spectrometer* USB4000, char paramNumber); // 0x05
extern STATUS writeConfig(spectrometer* USB4000, char paramNumber, char* writeASCII); // 0x06 writeASCII must be 16 bytes long
extern STATUS setTriggerMode(spectrometer* USB4000, TRIGGER triggerMode); // 0x0A

// I2C Functions - Not Implemented
/*
char queryNumAccessories(spectrometer* USB4000); // 0x0B
const char* queryAccessoryIDs(spectrometer* USB4000); // 0x0C
STATUS detectPlugins(spectrometer* USB4000); // 0x0D;
const char* readI2C(spectrometer* USB4000, char I2CAddress, char numBytes); // 0x60
STATUS writeI2C(spectrometer* USB4000, char I2CAddress, char numBytes, char* writeBytes); // 0x61
const char* SPIIO(spectrometer* USB4000,char numBytes, char writeBytes); // 0x62
*/

// Read and Write Microcontroller Registers.  Very raw functions, RTFM.
extern STATUS writeRegister(spectrometer* USB4000,char registerValue, char dataLSB, char dataMSB); // 0x6A
extern STATUS readRegister(spectrometer* USB4000,char registerValue); // 0x6B

// Status Query Functions
extern double readPCBTemp(spectrometer* USB4000); // 0x6C
extern STATUS updateStatus(spectrometer* USB4000); // 0xFE
extern void printStatus(spectrometer* USB4000);

// Data Read and Calibration Functions
extern STATUS updateWavelengthCalibrationCoefficients(spectrometer* USB4000);
extern void readDarkSpectra(spectrometer* USB4000, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds, unsigned short boxcar);
extern void readRefSpectra(spectrometer* USB4000, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds,unsigned short boxcar);
extern specSample* copySample(specSample* source,unsigned short numPixels);
extern specSample* getSample(spectrometer* USB4000, unsigned int numScansPerSample, unsigned int delayBetweenScansInMicroSeconds,unsigned short boxcar); // 0x09

#endif


/* SEASPeripheralCommands.h - Details all the functions used to control the Pumps, Lamp, and Heater via LON commands.  Also includes command wrappers for GUI and method file commands.
 *
 * By: Michael Lindemuth
 */

#ifndef SEASPERIPHCOMM_H
#define SEASPERIPHCOMM_H

#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include "GUIprotocol.h"
#include "LONprotocol.h"
#include "LONDispatch.h"

typedef struct PUMPSTATUS{
    unsigned char pumpID;
    unsigned char power;
    unsigned int RPM;
}pumpStatus_s;

typedef struct HEATERSTATUS{
    unsigned char heaterID;
    unsigned char power;
    float setTemperature;
    float currentTemperature;
}heaterStatus_s;

typedef struct CTDREADINGS{
    float conductivity;
    float temperature;
    float pressure;
    float soundVelocity;
}CTDreadings_s;


// Base Functions
void pumpOn(unsigned char pumpID);
void pumpOff(unsigned char pumpID);
void setPumpRPM(unsigned char pumpID, unsigned int RPM);
void lampOn();
void lampOff();
void heaterOn(unsigned char heaterID);
void heaterOff(unsigned char heaterID);
void setHeaterTemp(unsigned char heaterID,float temperature);

// Status Functions
pumpStatus_s* getPumpStatus(unsigned char pumpID);
float getHeaterCurrentTemperature(unsigned char heaterID);
heaterStatus_s* getHeaterStatus(unsigned char heaterID);
unsigned char getLampStatus();
float getBatteryVoltage();
CTDreadings_s* getCTDValues();

// GUI Protocol Wrappers
void receiveGetPumpStatus(int connection, char* command);
void receiveGetHeaterStatus(int connection, char* command);
void receiveGetLampStatus(int connection, char* command);
void receiveGetBatteryVoltage(int connection, char* command);
void receiveGetCTDValues(int connection, char* command);

// Method Wrappers
void methodPumpOn(unsigned long argc, void* argv);
void methodPumpOff(unsigned long argc, void* argv);
void methodLampOn(unsigned long argc, void* argv);
void methodLampOff(unsigned long argc, void* argv);
void methodHeaterOn(unsigned long argc, void* argv);
void methodHeaterOff(unsigned long argc, void* argv);

#endif

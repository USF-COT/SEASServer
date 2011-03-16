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
#include "parseGUI.h"
#include "A2D-MCP3424.h"

#define MAX_NUM_PUMPS 6
#define MAXPUMPFREQHZ 1200

#define CTD_NODE_GPIO 59
#define LON_INT_GPIO 61
#define PUMP_A_GPIO 63
#define PUMP_B_GPIO 65
#define PUMP_C_GPIO 67
#define HEAT_GPIO 69
#define SPARE_GPIO 71

typedef struct PUMPSTATUS{
    unsigned char pumpID;
    unsigned char power;
    unsigned int percent;
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

typedef struct PERIPHSTATUSES{
    BOOL pumpsStatus[MAX_NUM_PUMPS];
    BOOL heaterStatus;
    BOOL CTDStatus;
}periphStatuses_s;

// Init Status Function Must Be Called Before Anything Else Below
void initPeripheralStatuses();

// LON Power Management Monitor Functions
void enableLONPowerManagement();
void disableLONPowerManagement();

// Base Functions
void pumpOn(unsigned char pumpID);
void pumpOff(unsigned char pumpID);
void setPumpPercent(unsigned char pumpID, unsigned int percent);
void lampOn();
void lampOff();
void heaterOn(unsigned char heaterID);
void heaterOff(unsigned char heaterID);
//void CTDOn();
//void CTDOff();
void setHeaterTemp(unsigned char heaterID,float temperature);

// Status Functions
pumpStatus_s* getPumpStatus(unsigned char pumpID);
float getHeaterCurrentTemperature(unsigned char heaterID);
heaterStatus_s* getHeaterStatus(unsigned char heaterID);
unsigned char getLampStatus();
float getBatteryVoltage();
CTDreadings_s* getCTDValues();

// GUI Protocol Wrappers
void receiveSetPumpControl(int connection, char* command);
void receiveSetPumpPercent(int connection, char* command);
void receiveSetLampControl(int connection, char* command);
void receiveSetHeaterControl(int connection, char* command);
void receiveSetHeaterTemp(int connection, char* command);

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

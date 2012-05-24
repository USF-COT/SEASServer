/* SEASPeripheralCommands.h - Details all the functions used to control the Pumps, Lamp, and Heater via LON commands.  Also includes command wrappers for GUI and method file commands.
 *
 * By: Michael Lindemuth
 */

#ifndef SEASPERIPHCOMM_H
#define SEASPERIPHCOMM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include "GUIprotocol.h"
#include "LONprotocol.h"
#include "LONDispatch.h"
#include "parseGUI.h"
#include "benchConfig.h"

#define MAX_NUM_PUMPS 6
#define MAX_NUM_PUMP_LON_NODES 3
#define NUM_PUMPS_PER_LON 2
#define MAX_PUMP_PERCENT 1000

#define GPIO_EXP_FMT "echo %d > /sys/class/gpio/export"
#define GPIO_OUT_FMT "echo out > /sys/class/gpio/gpio%d/direction"
#define GPIO_VAL_FMT "echo %d > /sys/class/gpio/gpio%d/value"

#define CTD_NODE_GPIO 186 
#define LON_INT_GPIO 170
#define PUMP_A_GPIO 146
#define PUMP_B_GPIO 147
#define PUMP_C_GPIO 144
#define HEAT_GPIO 145
#define SPARE_GPIO 10

typedef enum PMPERIPH{PMCTD,PMLONHEAD,PMPUMP,PMHEAT}pmPeriph;

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
    time_t t;
    float conductivity;
    float temperature;
    float pressure;
    float soundVelocity;
}CTDreadings_s;

typedef struct PERIPHSTATUSES{
    BOOL CTDStatus;
    BOOL LONHead;
    BOOL pumpStatus[MAX_NUM_PUMPS];
    uint16_t pumpPercents[MAX_NUM_PUMPS];
    BOOL heaterStatus;
    float heaterSetTemp;
}periphStatuses_s;

void initPeripherals();

// Base Functions
void pumpOn(unsigned char pumpID);
void pumpOff(unsigned char pumpID);
void setPumpPercent(unsigned char pumpID, uint16_t percent);
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
void receiveGetCTDValues(int connection, char* command);
void receiveGetTemperatureValue(int connection, char* command);

// Method Wrappers
void methodPumpOn(unsigned long argc, void* argv);
void methodPumpOff(unsigned long argc, void* argv);
void methodLampOn(unsigned long argc, void* argv);
void methodLampOff(unsigned long argc, void* argv);
void methodHeaterOn(unsigned long argc, void* argv);
void methodHeaterOff(unsigned long argc, void* argv);

#ifdef __cplusplus
 }
#endif

#endif

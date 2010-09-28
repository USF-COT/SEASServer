/* SEASPeripheralCommands.h - Details all the functions used to control the Pumps, Lamp, and Heater via LON commands.  Also includes command wrappers for GUI and method file commands.
 *
 * By: Michael Lindemuth
 */

#ifndef SEASPERIPHCOMM_H
#define SEASPERIPHCOMM_H

#include <string.h>
#include <syslog.h>
#include "LONprotocol.h"
#include "LONDispatch.h"

// Base Functions
void pumpOn(unsigned char pumpID);
void pumpOff(unsigned char pumpID);
void setPumpRPM(unsigned char pumpID, unsigned int RPM);
void lampOn();
void lampOff();
void heaterOn(unsigned char heaterID);
void heaterOff(unsigned char heaterID);
void setHeaterTemp(unsigned char heaterID,float temperature);

// GUI Protocol Wrappers

// Method Wrappers
void methodPumpOn(unsigned long argc, void* argv);
void methodPumpOff(unsigned long argc, void* argv);
void methodLampOn(unsigned long argc, void* argv);
void methodLampOff(unsigned long argc, void* argv);
void methodHeaterOn(unsigned long argc, void* argv);
void methodHeaterOff(unsigned long argc, void* argv);

#endif

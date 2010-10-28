#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "config.h"
#include "GUIprotocol.h"
#include <sys/socket.h>

// GUI Handlers
void receiveWaveSetCommand(int connection, char* command);
void sendWaveParameters(int connection, char* command);
void receiveParameterBytes(int connection, char* command);
void sendSpecParameters(int connection, char* command);

// Method Script Handlers
void methodSetSpectrometerParameters(unsigned long argc, void* argv);
void methodSetAbsorbanceWavelengths(unsigned long argc, void* argv);
void methodSetNonAbsorbanceWavelength(unsigned long argc, void* argv);
void methodSetDwell(unsigned long argc, void* argv);

void writeConfigChanges(int connection, char* command);

#endif

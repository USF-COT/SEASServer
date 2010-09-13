#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "config.h"
#include "GUIprotocol.h"
#include <sys/socket.h>

void receiveWaveSetCommand(int connection, char* command);
void sendWaveParameters(int connection, char* command);
void receiveParameterBytes(int connection, char* command);
void sendSpecParameters(int connection, char* command);

void writeConfigChanges(int connection, char* command);

#endif

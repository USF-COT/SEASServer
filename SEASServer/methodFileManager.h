#ifndef METHODFILEMANAGER_H
#define METHODFILEMANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include "parseGUI.h"
#include "GUIprotocol.h"
#include "MethodParser/MethodParser.tab.h"
#include "MethodParser/MethodNodesTable.h"
#include "socketUtils.h"
#include "runProtocolManager.h"

extern FILE* yyin;

/* ReceiveMethodFile - Responsible for parsing filename and contents from a character buffer
 * in a specified format.  Lines are denoted by the reserved '\n' character.
 * 
 * Buffer format:
 * Line 1: Filename
 * Line 2-EOF: Contents
 */
void receiveMethodFile(int connection, char* command);
void sendMethodFileList(int connection, char* command);
void sendMethodFile(int connection, char* command);
void receiveSetCurrentMethodFile(int connection, char* command);
void receiveDeleteMethodFile(int connection, char* command);
char* getActiveMethodFilename();
void sendActiveMethodFile(int connection, char* command);
void receiveExecuteMethod(int connection, char* command);
void receiveTerminateMethod(int connection, char* command);
#endif

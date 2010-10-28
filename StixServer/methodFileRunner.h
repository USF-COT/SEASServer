/* MethodFileRunner.h - Responsible for handling commands to execute and terminate the default method file commands.  Can only run one method file at a time by design.  Any subsequent calls to start a method filewhile one is already running will be ignored.
 * 
 * By: Michael Lindemuth
 */

#ifndef METHODFILERUNNER_H
#define METHODFILERUNNER_H

#include <syslog.h>
#include <pthread.h>
#include "MethodNodesTable.h"
#include "MethodParser.tab.h"

extern FILE* yyin;

// Generic Methods
void executeMethodFile();
void terminateMethodFile();

// GUI specific wrapper methods
void receiveExecuteMethod(int connection, char* command);
void receiveTerminateMethod(int connection, char* command);

#endif

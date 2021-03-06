/* MethodFileRunner.h - Responsible for handling commands to execute and terminate the default method file commands.  Can only run one method file at a time by design.  Any subsequent calls to start a method filewhile one is already running will be ignored.
 * 
 * By: Michael Lindemuth
 */

#ifndef METHODFILERUNNER_H
#define METHODFILERUNNER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <syslog.h>
#include <pthread.h>
#include "MethodParser/MethodNodesTable.h"
#include "MethodParser/MethodParser.tab.h"
#include "methodFileManager.h"

extern FILE* yyin;

// Generic Methods
void executeMethodFile();
void terminateMethodFile();
void terminateMethodAndWait();

void waitOnMethodFile();

#ifdef __cplusplus
 }
#endif

#endif

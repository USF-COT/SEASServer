/* DATAFILEMANAGER: Contains methods to open a new data file, write both concentration and full spectrum data, retrieve data, and close the current data file.
 *
 * By: Michael Lindemuth
 */

#ifndef DATAFILEMAN_H
#define DATAFILEMAN_H

#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include "pthread.h"
#include "sqlite3.h"
#include "USB4000Manager.h"
#include "salinity.h"
#include "SEASPeripheralCommands.h"

#define DATAFILEPATH "/home/datauser/data/"

// File Management Functions
void openDataFile();
void closeDataFile();

// Write Functions
void writeConfigToDB();
void writeConcData(unsigned char specID);
void writeFullSpec(unsigned char specID);
void writepHToDB(unsigned char,float,unsigned char,float*,struct CTDREADINGS);

// Method Wrapper Functions
void methodOpenDataFile(unsigned long argc, void* argv);
void methodCloseDataFile(unsigned long argc, void* argv);
void methodWriteConcData(unsigned long argc, void* argv);
void methodWriteFullSpec(unsigned long argc, void* argv);

#endif

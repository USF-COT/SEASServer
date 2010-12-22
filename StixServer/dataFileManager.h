/* DATAFILEMANAGER: Contains methods to open a new data file, write both concentration and full spectrum data, retrieve data, and close the current data file.
 *
 * By: Michael Lindemuth
 */

#include <stdio.h>
#include <time.h>
#include <syslog.h>

#define DATAFILEPATH "/media/card/data/"

// File Management Functions
void openDataFile();
void closeDataFile();

// Write Functions
void writeConcData();
void writeFullSpec();

// Method Wrapper Functions
void methodOpenDataFile(unsigned long argc, void* argv);
void methodCloseDataFile(unsigned long argc, void* argv);
void methodWriteConcData(unsigned long argc, void* argv);
void methodWriteFullSpec(unsigned long argc, void* argv);

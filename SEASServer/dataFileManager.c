#include "dataFileManager.h"

static sqlite3* db = NULL;
static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

void applyDBSchema(sqlite3* db){
    // TODO: Create schema statements here
}

// File Management Functions
void openDataFile(){
    char filePath[128];

    if(!db){
        pthread_mutex_lock(&dataMutex);
        sprintf(filePath,"%s%u.db",DATAFILEPATH,(unsigned int)time(NULL));        
        syslog(LOG_DAEMON|LOG_INFO,"Opening data file @: %s.",filePath);
        if(sqlite3_open(filePath,&db) != SQLITE_OK){
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to open data file @: %s.",filePath);
        } else {
            applyDBSchema(db);
        }
        pthread_mutex_unlock(&dataMutex);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Tried to open a data file when one is already open.");
    }
}
void closeDataFile(){
    if(currDataFile){
        syslog(LOG_DAEMON|LOG_INFO,"Closing data file.");
        fclose(currDataFile);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Tried to close data file when one is not open.");
    }
}

// Write Functions
void writeConcData(){
    if(currDataFile){
        // Write Concentration Data Here
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to write concentration data because no data file is open.");
    }
}

void writeFullSpec(){
    if(currDataFile){
        // Write Full Spectrum Data
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to write full spectrum data because no data file is open.");
    }
}

// Method Wrapper Functions
void methodOpenDataFile(unsigned long argc, void* argv){

}

void methodCloseDataFile(unsigned long argc, void* argv){

}

void methodWriteConcData(unsigned long argc, void* argv){

}

void methodWriteFullSpec(unsigned long argc, void* argv){

}


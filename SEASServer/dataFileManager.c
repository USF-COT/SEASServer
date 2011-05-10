#include "dataFileManager.h"

static sqlite3 *db = NULL;
static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

void applyDBSchema(sqlite3* db){
    char *query;
    char *errMsg;

    // Create configs table
    query = "CREATE  TABLE  IF NOT EXISTS \"main\".\"configs\" (\"time\" INTEGER PRIMARY KEY NOT NULL  UNIQUE, \"analyte_name\" TEXT NOT NULL , \"dwell\" INTEGER NOT NULL , \"scans_per_sample\" INTEGER NOT NULL , \"boxcar\" INTEGER NOT NULL , \"reference_spectrum\" BLOB NOT NULL , \"absorbance_wavelength_1\" REAL NOT NULL , \"absorbance_wavelength_2\" REAL, \"absorbance_wavelength_3\" REAL, \"absorbance_wavelength_4\" REAL, \"absorbance_wavelength_5\" REAL, \"absorbance_wavelength_6\" REAL, \"absorbance_wavelength_7\" REAL, \"absorbance_wavelength_8\" REAL, \"absorbance_wavelength_9\" REAL, \"non-absorbing_wavelength\" REAL NOT NULL )";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Configs Table Query Failed: %s", errMsg);
    }

    // Create concentrations table
    query = "CREATE TABLE IF NOT EXISTS \"main\".\"concentrations\" (\"time\" INTEGER PRIMARY KEY NOT NULL UNIQUE, \"integration_time\" INTEGER NOT NULL, \"concentration_1\" REAL NOT NULL, \"concentration_2\" REAL,\"concentration_3\" REAL,\"concentration_4\" REAL,\"concentration_5\" REAL,\"concentration_6\" REAL,\"concentration_7\" REAL,\"concentration_8\" REAL,\"concentration_9\" REAL,\"original_count_1\" REAL NOT NULL,\"original_count_2\" REAL,\"original_count_3\" REAL,\"original_count_4\" REAL,\"original_count_5\" REAL,\"original_count_6\" REAL,\"original_count_7\" REAL,\"original_count_8\" REAL,\"original_count_9\" REAL,\"measured_absorbance_1\" REAL NOT NULL,\"measured_absorbance_2\" REAL,\"measured_absorbance_3\" REAL,\"measured_absorbance_4\" REAL,\"measured_absorbance_5\" REAL,\"measured_absorbance_6\" REAL,\"measured_absorbance_7\" REAL,\"measured_absorbance_8\" REAL,\"measured_absorbance_9\" REAL,\"original_count_non_absorbing\" REAL NOT NULL, \"measure_absorbance_non_absorbing\" REAL NOT NULL,\"conductivity\" REAL NOT NULL, \"temperature\" REAL NOT NULL, \"depth\" REAL NOT NULL,\"salinity\" REAL NOT NULL,\"heater_temperature\" REAL NOT NULL )";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Table Query Failed: %s", errMsg);
    }

    // Create full_spectrums table
    query = "CREATE TABLE IF NOT EXISTS \"main\".\"full_spectrums\" (\"time\" INTEGER PRIMARY KEY NOT NULL UNIQUE, \"integration_time\" INTEGER NOT NULL, \"sample_spectrum\" BLOB NOT NULL, \"conductivity\" REAL NOT NULL, \"temperature\" REAL NOT NULL, \"depth\" REAL NOT NULL,\"salinity\" REAL NOT NULL,\"heater_temperature\" REAL NOT NULL )";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Full_Spectrums Table Query Failed: %s", errMsg);
    }
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
    if(db){
        syslog(LOG_DAEMON|LOG_INFO,"Closing data file.");
        pthread_mutex_lock(&dataMutex);
        sqlite3_close(db);
        db = NULL;
        pthread_mutex_unlock(&dataMutex);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Tried to close data file when one is not open.");
    }
}

// Write Functions
void writeConcData(){
    if(db){
        // Write Concentration Data Here
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to write concentration data because no data file is open.");
    }
}

void writeFullSpec(){
    if(db){
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


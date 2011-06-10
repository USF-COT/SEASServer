#include "dataFileManager.h"

static sqlite3 *db = NULL;
static uint64_t currConfigID[NUM_SPECS];
static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

void applyDBSchema(sqlite3* db){
    char *query;
    char *errMsg;

    // Create configs table
    query = "CREATE  TABLE  IF NOT EXISTS \"main\".\"configs\" (\"config_id\" INTEGER PRIMARY KEY NOT NULL, \"time\" INTEGER NOT NULL, \"spec_id\" INTEGER NOT NULL, \"analyte_name\" TEXT NOT NULL , \"integration_time\" INTEGER NOT NULL, \"dwell\" INTEGER NOT NULL , \"scans_per_sample\" INTEGER NOT NULL , \"boxcar\" INTEGER NOT NULL , \"reference_spectrum\" BLOB , \"absorbance_wavelength_1\" REAL, \"absorbance_wavelength_2\" REAL, \"absorbance_wavelength_3\" REAL, \"absorbance_wavelength_4\" REAL, \"absorbance_wavelength_5\" REAL, \"absorbance_wavelength_6\" REAL, \"absorbance_wavelength_7\" REAL, \"absorbance_wavelength_8\" REAL, \"absorbance_wavelength_9\" REAL, \"non-absorbing_wavelength\" REAL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Configs Table Query Failed: %s", errMsg);
    }

    // Create concentrations table
    query = "CREATE TABLE IF NOT EXISTS \"main\".\"concentrations\" (\"time\" INTEGER NOT NULL, \"config_id\" INTEGER NOT NULL DEFAULT 0 REFERENCES configs(config_id) ON UPDATE CASCADE ON DELETE SET DEFAULT, \"concentration_1\" REAL NOT NULL, \"concentration_2\" REAL,\"concentration_3\" REAL,\"concentration_4\" REAL,\"concentration_5\" REAL,\"concentration_6\" REAL,\"concentration_7\" REAL,\"concentration_8\" REAL,\"concentration_9\" REAL,\"original_count_1\" REAL NOT NULL,\"original_count_2\" REAL,\"original_count_3\" REAL,\"original_count_4\" REAL,\"original_count_5\" REAL,\"original_count_6\" REAL,\"original_count_7\" REAL,\"original_count_8\" REAL,\"original_count_9\" REAL,\"measured_absorbance_1\" REAL NOT NULL,\"measured_absorbance_2\" REAL,\"measured_absorbance_3\" REAL,\"measured_absorbance_4\" REAL,\"measured_absorbance_5\" REAL,\"measured_absorbance_6\" REAL,\"measured_absorbance_7\" REAL,\"measured_absorbance_8\" REAL,\"measured_absorbance_9\" REAL,\"original_count_non_absorbing\" REAL NOT NULL, \"measured_absorbance_non_absorbing\" REAL NOT NULL,\"conductivity\" REAL NOT NULL, \"temperature\" REAL NOT NULL, \"depth\" REAL NOT NULL,\"salinity\" REAL NOT NULL,\"heater_temperature\" REAL NOT NULL, PRIMARY KEY(time,config_id) )";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Table Query Failed: %s", errMsg);
    }

    // Create full_spectrums table
    query = "CREATE TABLE IF NOT EXISTS \"main\".\"full_spectrums\" (\"time\" INTEGER NOT NULL, \"config_id\" INTEGER NOT NULL DEFAULT 0 REFERENCES configs(config_id) ON UPDATE CASCADE ON DELETE SET DEFAULT, \"sample_spectrum\" BLOB NOT NULL, \"conductivity\" REAL NOT NULL, \"temperature\" REAL NOT NULL, \"depth\" REAL NOT NULL,\"salinity\" REAL NOT NULL,\"heater_temperature\" REAL NOT NULL, PRIMARY KEY(time, config_id))";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Full_Spectrums Table Query Failed: %s", errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.conf_ana ON configs (analyte_name)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Configs Analyte Name Index Query Failed: %s", errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.conc_depth ON concentrations (depth)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Depth Index Query Failed: %s", errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.conc_temp ON concentrations (temperature)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Temperature Index Query Failed: %s", errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.conc_sal ON concentrations (salinity)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Salinity Index Query Failed: %s", errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.fs_depth ON full_spectrums (depth)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Full Spectrums Depth Index Query Failed: %s",errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.fs_temp ON full_spectrums (temperature)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Full Spectrums Temperature Index Query Failed: %s",errMsg);
    }

    query = "CREATE INDEX IF NOT EXISTS main.fs_sal ON full_spectrums (salinity)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Full Spectrums Salinity Index Query Failed: %s",errMsg);
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
void writeConfig(){
    int i,j,configIndex=0;
    
    specConfig* config = NULL;
    specSample* refSample = NULL;
    // If MAX_ABS_WAVES in config.h is changed, this line must be changed manually!
    const char* insertStmt = "INSERT INTO main.configs (time,spec_id,analyte_name,integration_time,dwell,scans_per_sample,boxcar,reference_spectrum,absorbance_wavelength_1,absorbance_wavelength_2,absorbance_wavelength_3,absorbance_wavelength_4,absorbance_wavelength_5,absorbance_wavelength_6,absorbance_wavelength_7,absorbance_wavelength_8,absorbance_wavelength_9,non-absorbing_wavelength) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;
	
    // Do nothing if a data file is not currently open
    if(!db){
        syslog(LOG_DAEMON|LOG_INFO,"Configuration change will not be logged because a data file is not currently open.");
        return;
    }
    
    if(sqlite3_prepare(db,insertStmt,1024,&pStmt) == SQLITE_OK){
        for(i=0; i < NUM_SPECS; i++){
            config = getConfigCopy(i);
            refSample = getRefSample(i);
            if(config){
                // Fill prepared statement
                sqlite3_bind_int64(pStmt,1,(int)time(NULL));
                sqlite3_bind_int(pStmt,2,i);
                sqlite3_bind_text(pStmt,3,config->waveParameters.analyteName,strlen(config->waveParameters.analyteName),SQLITE_TRANSIENT);
                sqlite3_bind_int(pStmt,4,config->specParameters.integrationTime);
                sqlite3_bind_int64(pStmt,5,config->dwell);
                sqlite3_bind_int(pStmt,6,config->specParameters.scansPerSample);
                sqlite3_bind_int(pStmt,7,config->specParameters.boxcarSmoothing);
                if(refSample){
                    sqlite3_bind_blob(pStmt,8,refSample->pixels,getNumPixels(i)*sizeof(float),SQLITE_TRANSIENT);
                } else {
                    sqlite3_bind_null(pStmt,8);
                }
                
                for(j=0; j < MAX_ABS_WAVES; j++){
                    if(config->waveParameters.absorbingWavelength[j] > 0){
                        sqlite3_bind_double(pStmt,9+j,(double)config->waveParameters.absorbingWavelength[j]);
                    } else {
                        sqlite3_bind_null(pStmt,9+j);
                    }
                } 
                
                if(config->waveParameters.nonAbsorbingWavelength > 0){
                    sqlite3_bind_double(pStmt,9+j,(double)config->waveParameters.nonAbsorbingWavelength);
                } else {
                    sqlite3_bind_null(pStmt,9+j);
                }
                
                // Run Prepared Statement
                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to log to SQLite database for spectrometer %d.  Error: %s",i,sqlite3_errmsg(db));
                }
                
                // Reset Prepared Statement for Next Call
                sqlite3_reset(pStmt);
                
                // housekeeping!
                deallocateSample(refSample);
                freeSpecConfig(config);
            } else {
                syslog(LOG_DAEMON|LOG_ERR, "Unable to retrieve config spec for spectrometer %d",i);
            }
            currConfigID[i] = sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(pStmt);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to create prepared statement in writeConfig() method.");
    }
    
}

void writeConcData(){
    time_t t;
    float heatTemp;
    CTDreadings_s* ctd;    
    float* conc;
    float* abs;
    float* counts;

    if(db){
        // Get data for row
        t = time(NULL);
        heatTemp = getHeaterCurrentTemperature(1);
        ctd = getCTDValues();
        
        

        free(ctd);
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
    openDataFile();
}

void methodCloseDataFile(unsigned long argc, void* argv){
    closeDataFile();
}

void methodWriteConcData(unsigned long argc, void* argv){
    writeConcData();
}

void methodWriteFullSpec(unsigned long argc, void* argv){
    writeFullSpec();
}


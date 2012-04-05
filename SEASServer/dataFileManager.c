#include "dataFileManager.h"


static sqlite3 *db = NULL;
static int64_t currConfigID[NUM_SPECS];
static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

void applyDBSchema(sqlite3* db){
    char *query;
    char *errMsg;

    // Create configs table
    query = "CREATE  TABLE  IF NOT EXISTS main.configs (\"config_id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,  \"time\" INTEGER NOT NULL, \"spec_id\" INTEGER NOT NULL, \"analyte_name\" TEXT NOT NULL , \"integration_time\" INTEGER NOT NULL, \"dwell\" INTEGER NOT NULL , \"scans_per_sample\" INTEGER NOT NULL , \"boxcar\" INTEGER NOT NULL , \"reference_spectrum\" BLOB , \"absorbance_wavelength_1\" REAL, \"absorbance_wavelength_2\" REAL, \"absorbance_wavelength_3\" REAL, \"absorbance_wavelength_4\" REAL, \"absorbance_wavelength_5\" REAL, \"absorbance_wavelength_6\" REAL, \"absorbance_wavelength_7\" REAL, \"absorbance_wavelength_8\" REAL, \"absorbance_wavelength_9\" REAL, \"non_absorbing_wavelength\" REAL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Configs Table Query Failed: %s", errMsg);
    }
    
    // Create ctd_readings table
    query = "CREATE TABLE IF NOT EXISTS main.ctd_readings(ctd_reading_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,time INTEGER NOT NULL,conductivity REAL NOT NULL,temperature REAL NOT NULL,depth REAL NOT NULL,salinity REAL NOT NULL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create ctd_readings Table Query Failed: %s",errMsg);
    }

    // Create concentrations table
    query = "CREATE TABLE IF NOT EXISTS main.concentrations (\"time\" INTEGER NOT NULL, \"config_id\" INTEGER NOT NULL DEFAULT 0 REFERENCES configs(config_id) ON UPDATE CASCADE ON DELETE SET DEFAULT, \"concentration_1\" REAL NOT NULL, \"concentration_2\" REAL,\"concentration_3\" REAL,\"concentration_4\" REAL,\"concentration_5\" REAL,\"concentration_6\" REAL,\"concentration_7\" REAL,\"concentration_8\" REAL,\"concentration_9\" REAL,\"original_count_1\" REAL NOT NULL,\"original_count_2\" REAL,\"original_count_3\" REAL,\"original_count_4\" REAL,\"original_count_5\" REAL,\"original_count_6\" REAL,\"original_count_7\" REAL,\"original_count_8\" REAL,\"original_count_9\" REAL,\"measured_absorbance_1\" REAL NOT NULL,\"measured_absorbance_2\" REAL,\"measured_absorbance_3\" REAL,\"measured_absorbance_4\" REAL,\"measured_absorbance_5\" REAL,\"measured_absorbance_6\" REAL,\"measured_absorbance_7\" REAL,\"measured_absorbance_8\" REAL,\"measured_absorbance_9\" REAL,\"original_count_non_absorbing\" REAL NOT NULL, \"measured_absorbance_non_absorbing\" REAL NOT NULL,ctd_reading_id INTEGER NOT NULL DEFAULT 0 REFERENCES ctd_readings(ctd_reading_id) ON UPDATE CASCADE ON DELETE SET DEFAULT,\"heater_temperature\" REAL NOT NULL, PRIMARY KEY(time,config_id) )";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Table Query Failed: %s", errMsg);
    }

    // Create full_spectrums table
    query = "CREATE TABLE IF NOT EXISTS main.full_spectrums (\"time\" INTEGER NOT NULL, \"config_id\" INTEGER NOT NULL DEFAULT 0 REFERENCES configs(config_id) ON UPDATE CASCADE ON DELETE SET DEFAULT, \"sample_spectrum\" BLOB NOT NULL, ctd_reading_id INTEGER NOT NULL DEFAULT 0 REFERENCES ctd_readings(ctd_reading_id) ON UPDATE CASCADE ON DELETE SET DEFAULT ,\"heater_temperature\" REAL NOT NULL, PRIMARY KEY(time, config_id))";
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
        
        writeConfigToDB();  // Log Initial Configuration
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
void writeConfigToDB(){
    int i,j,configIndex=0;
    
    specConfig* config = NULL;
    specSample* refSample = NULL;
    // If MAX_ABS_WAVES in config.h is changed, this line must be changed manually!
    const char* insertStmt = "INSERT INTO main.configs (time,spec_id,analyte_name,integration_time,dwell,scans_per_sample,boxcar,reference_spectrum,absorbance_wavelength_1,absorbance_wavelength_2,absorbance_wavelength_3,absorbance_wavelength_4,absorbance_wavelength_5,absorbance_wavelength_6,absorbance_wavelength_7,absorbance_wavelength_8,absorbance_wavelength_9,non_absorbing_wavelength) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;
	
    // Do nothing if a data file is not currently open
    if(!db){
        syslog(LOG_DAEMON|LOG_INFO,"Configuration change will not be logged because a data file is not currently open.");
        return;
    }
    
    pthread_mutex_lock(&dataMutex);
    if(sqlite3_prepare(db,insertStmt,1024,&pStmt,NULL) == SQLITE_OK){
        for(i=0; i < NUM_SPECS; i++){
            config = getConfigCopy(i);
            refSample = getRefSample(i);
            if(config){
                // Fill prepared statement
                sqlite3_bind_int64(pStmt,1,(int64_t)time(NULL));
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
                    if(config->waveParameters.absorbingWavelengths[j] > 0){
                        sqlite3_bind_double(pStmt,9+j,(double)config->waveParameters.absorbingWavelengths[j]);
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
                
                // housekeeping!
                deallocateSample(&refSample);
                freeSpecConfig(config);
            } else {
                syslog(LOG_DAEMON|LOG_ERR, "Unable to retrieve config spec for spectrometer %d",i);
            }
            currConfigID[i] = sqlite3_last_insert_rowid(db);
            
            // Reset Prepared Statement for Next Call
            sqlite3_reset(pStmt);
        }
        sqlite3_finalize(pStmt);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to create prepared statement in writeConfig() method.  Error: %s", sqlite3_errmsg(db));
    }
    pthread_mutex_unlock(&dataMutex);
    
}

uint64_t writeCTDData(){
    uint64_t retVal = 0;
    CTDreadings_s* ctd;
    char* insertStmt = "INSERT INTO ctd_readings (time,conductivity,temperature,depth,salinity) VALUES (?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;
    
    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,insertStmt,256,&pStmt,NULL) == SQLITE_OK){
            ctd = getCTDValues();
            
            sqlite3_bind_int64(pStmt,1,(int64_t)time(NULL));
            sqlite3_bind_double(pStmt,2,(double)ctd->conductivity);
            sqlite3_bind_double(pStmt,3,(double)ctd->temperature);
            sqlite3_bind_double(pStmt,4,(double)ctd->pressure);
            sqlite3_bind_double(pStmt,5,(double)computeSalinity(ctd->conductivity,ctd->temperature,ctd->pressure));
            
            if(sqlite3_step(pStmt) == SQLITE_ERROR){
                syslog(LOG_DAEMON|LOG_ERR,"Unable to log CTD values to SQLite database.  Error: %s",sqlite3_errmsg(db));
            } else {
                retVal = sqlite3_last_insert_rowid(db);
            }
            
            free(ctd);
            sqlite3_finalize(pStmt);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Cannot create prepared statement for writeCTDData() method.  Error: %s",sqlite3_errmsg(db));
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Cannot log CTD data when SQLite file is not open.");
    }
    pthread_mutex_unlock(&dataMutex);
    
    return retVal;
}

void writeConcData(){
    unsigned int i,j,stmtI=1;
    uint64_t ctd_id = 0;   
    float* conc;
    float* abs;
    float* counts;
    
    // If MAX_ABS_WAVES in config.h is changed, this line must be changed manually!
    const char* insertStmt = "INSERT INTO main.concentrations (time,config_id,concentration_1,concentration_2,concentration_3,concentration_4,concentration_5,concentration_6,concentration_7,concentration_8,concentration_9,original_count_1,original_count_2,original_count_3,original_count_4,original_count_5,original_count_6,,original_count_7,original_count_8,original_count_9,measured_absorbance_1,measured_absorbance_2,measured_absorbance_3,measured_absorbance_4,measured_absorbance_5,measured_absorbance_6,measured_absorbance_7,measured_absorbance_8,measured_absorbance_9,original_count_non_absorbing,measured_absorbance_non_absorbing,ctd_reading_id,heater_temperature) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;

    ctd_id = writeCTDData();
    
    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,insertStmt,2048,&pStmt,NULL) == SQLITE_OK){
            for(i=0; i < NUM_SPECS; i++){
                stmtI = 0;
                sqlite3_bind_int64(pStmt,stmtI++,(int64_t)time(NULL));
                sqlite3_bind_int64(pStmt,stmtI++,(int64_t)currConfigID[i]);
                
                conc = getConcentrations(i);
                if(!conc){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve concentrations.  Creating zero padded array.");
                    conc = calloc(MAX_ABS_WAVES+1,sizeof(float));
                }
                
                if(conc){
                    for(j=0; j < MAX_ABS_WAVES; j++){
                        if(conc[j] != 0){
                            sqlite3_bind_double(pStmt,stmtI++,(double)conc[j]);
                        } else {
                            sqlite3_bind_null(pStmt,stmtI++);
                        }
                    }
                } else {
                    syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate concentration array.");
                    sqlite3_finalize(pStmt);
                    pthread_mutex_unlock(&dataMutex);
                    return;
                }
                free(conc);
                
                abs = getAbsorbance(i);
                if(!abs){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve absorbance.  Creating zero padded array.");
                    abs = calloc(MAX_ABS_WAVES+1,sizeof(float));
                }
                
                if(abs){
                    for(j=0; j < MAX_ABS_WAVES; j++){
                        if(abs[j] != 0){
                            sqlite3_bind_double(pStmt,stmtI++,(double)abs[j]);
                        } else {
                            sqlite3_bind_null(pStmt,stmtI++);
                        }
                    }
                } else {
                    syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate absorbance array.");
                    sqlite3_finalize(pStmt);
                    pthread_mutex_unlock(&dataMutex);
                    return;
                }
                
                counts = getRawCounts(i);
                if(!counts){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve concentrations.  Creating zero padded array.");
                    counts = calloc(MAX_ABS_WAVES+1,sizeof(float));
                }    
                
                if(counts){
                    for(j=0; j < MAX_ABS_WAVES; j++){
                        if(counts[j] != 0){
                            sqlite3_bind_double(pStmt,stmtI++,(double)counts[j]);
                        } else {
                            sqlite3_bind_null(pStmt,stmtI++);
                        }
                    }
                } else {
                    syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate absorbance array.");
                    sqlite3_finalize(pStmt);
                    pthread_mutex_unlock(&dataMutex);
                    return;
                }
                
                // Log non_absorbing count and absorbance
                sqlite3_bind_double(pStmt,stmtI++,(double)counts[MAX_ABS_WAVES]);
                sqlite3_bind_double(pStmt,stmtI++,(double)abs[MAX_ABS_WAVES]);
                free(counts);
                free(abs);
                
                // Log CTD Reading ID (FINALLY)
                sqlite3_bind_int64(pStmt,stmtI++,ctd_id);
                
                // Log Heater Temperature
                sqlite3_bind_double(pStmt,stmtI,(double)getHeaterCurrentTemperature(1));
                
                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to log concentration to DB for spectrometer #%d.  Error: %s",i,sqlite3_errmsg(db));
                }
                
                // Reset Prepared Statement for Next Call
                sqlite3_reset(pStmt);
            }
            sqlite3_finalize(pStmt);
        } else {
            syslog(LOG_DAEMON|LOG_ERR, "Unable to create prepared statement for concentrations data.  Error: %s",sqlite3_errmsg(db));
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to write concentration data because no data file is open.");
    }
    pthread_mutex_unlock(&dataMutex);
}

void writeFullSpec(){
    unsigned int i;
    uint64_t ctd_id = 0;
    specSample* sample;
    
    char* insertStmt = "INSERT INTO full_spectrums (time,config_id,sample_spectrum,ctd_reading_id,heater_temperature) VALUES (?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;

    ctd_id = writeCTDData();
    
    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,insertStmt,1024,&pStmt,NULL) == SQLITE_OK){
            for(i=0; i < NUM_SPECS; i++){
                sample = getDefaultSample(i);
                
                if(sample){
                    sqlite3_bind_int64(pStmt,1,(uint64_t)time(NULL));
                    sqlite3_bind_int64(pStmt,2,currConfigID[i]);
                    sqlite3_bind_blob(pStmt,3,sample->pixels,getNumPixels(i)*sizeof(float),SQLITE_TRANSIENT);
                    sqlite3_bind_int64(pStmt,4,ctd_id);
                    sqlite3_bind_double(pStmt,5,getHeaterCurrentTemperature(i));
                    
                    if(sqlite3_step(pStmt) == SQLITE_ERROR){
                        syslog(LOG_DAEMON|LOG_ERR,"Error logging full spectrum prepared statement for spectrometer #%d.  Error: %s",i,sqlite3_errmsg(db));
                    }
                    deallocateSample(&sample);
                } else {
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve sample from spectrometer #%d.",i);
                }
                sqlite3_reset(pStmt);
            }
            sqlite3_finalize(pStmt);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to create prepared statement for full spectrum table.  Error: %s",sqlite3_errmsg(db));
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to write full spectrum data because no data file is open.");
    }
    pthread_mutex_unlock(&dataMutex);
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


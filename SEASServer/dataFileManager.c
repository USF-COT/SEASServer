#include "dataFileManager.h"

static sqlite3 *db = NULL;
static int64_t currConfigID[NUM_SPECS];
static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

void applyDBSchema(sqlite3* db){
    char *query;
    char *errMsg;

    // Create configs table
    query = "CREATE TABLE [configs] ([config_id] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[time] INTEGER  NOT NULL,[spec_id] INTEGER  NOT NULL,[analyte_name] TEXT  NOT NULL,[integration_time] INTEGER  NOT NULL,[dwell] INTEGER  NOT NULL,[scans_per_sample] INTEGER  NOT NULL,[boxcar] INTEGER  NOT NULL, [slopes] TEXT NOT NULL, [intercepts] TEXT NOT NULL, [reference_spectrum] BLOB  NULL, [wavelengths] BLOB NULL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Configs Table Query Failed: %s", errMsg);
    }

    // Create ctd_readings table
    query = "CREATE TABLE [ctd_readings] ([ctd_reading_id] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[time] INTEGER  NOT NULL,[conductivity] REAL  NOT NULL,[temperature] REAL  NOT NULL,[depth] REAL  NOT NULL,[salinity] REAL  NOT NULL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create ctd_readings Table Query Failed: %s",errMsg);
    }

    // Create wavelength sets table
    query = "CREATE TABLE [wavelength_sets] ([wavelength_set_id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,[time] INTEGER  NOT NULL,[config_id] INTEGER  NOT NULL,[non_absorbing_wavelength] REAL  NOT NULL,[original_count_non_absorbing] REAL  NOT NULL,[measured_absorbance_non_absorbing] REAL  NOT NULL,[ctd_reading_id] INTEGER  NOT NULL,[heater_temperature] REAL  NOT NULL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Wavelength Sets Table Query Failed: %s",errMsg);
    }

    // Create concentrations table
    query = "CREATE TABLE [concentrations] ([wavelength_set_id] INTEGER  NOT NULL,[wavelength] REAL  NOT NULL,[concentration] REAL  NOT NULL,[original_count] REAL  NOT NULL,[measured_absorbance] REAL  NOT NULL,PRIMARY KEY ([wavelength_set_id],[wavelength]))";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create Concentrations Table Query Failed: %s", errMsg);
    }

    query = "CREATE TABLE [pH_wavelengths] ([wavelength_set_id] INTEGER  NOT NULL,[wavelength] REAL  NOT NULL,[original_count] REAL  NOT NULL,[measured_absorbance] REAL  NOT NULL,PRIMARY KEY ([wavelength_set_id],[wavelength]))";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create pH Waves Table Query Failed: %s", errMsg);
    }

    query = "CREATE TABLE [pH] ([wavelength_set_id] INTEGER PRIMARY KEY NOT NULL, [pH] REAL NOT NULL)";
    if(sqlite3_exec(db,query,NULL,NULL,&errMsg) != SQLITE_OK){
        syslog(LOG_DAEMON|LOG_ERR, "SQLite Create pH Table Query Failed: %s", errMsg);
    }

    // Create full_spectrums table
    query = "CREATE TABLE [full_spectrums] ([time] INTEGER  NOT NULL,[config_id] INTEGER DEFAULT '0' NOT NULL,[sample_spectrum] BLOB  NOT NULL,[ctd_reading_id] INTEGER DEFAULT '0' NOT NULL,[heater_temperature] REAL  NOT NULL,PRIMARY KEY ([time],[config_id]))";
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
    unsigned int i,j;

    specConfig* config = NULL;
    float* slopes = NULL;
    char slopeString[1024];
    float* intercepts = NULL;
    char intString[1024];
    specSample* refSample = NULL;
    double* wavelengths = NULL;
    // If MAX_ABS_WAVES in config.h is changed, this line must be changed manually!
    const char* insertStmt = "INSERT INTO main.configs (time,spec_id,analyte_name,integration_time,dwell,scans_per_sample,boxcar,slopes,intercepts,reference_spectrum,wavelengths) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
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
            wavelengths = getWavelengths(i);
            if(config){
                // Fill prepared statement
                sqlite3_bind_int64(pStmt,1,(int64_t)time(NULL));
                sqlite3_bind_int(pStmt,2,i);
                sqlite3_bind_text(pStmt,3,config->waveParameters.analyteName,strlen(config->waveParameters.analyteName),SQLITE_TRANSIENT);
                sqlite3_bind_int(pStmt,4,config->specParameters.integrationTime);
                sqlite3_bind_int64(pStmt,5,config->dwell);
                sqlite3_bind_int(pStmt,6,config->specParameters.scansPerSample);
                sqlite3_bind_int(pStmt,7,config->specParameters.boxcarSmoothing);
                
                // Store slopes
                slopes = getSlopes(i);
                intercepts = getIntercepts(i);
                if(slopes && intercepts){
                    sprintf(slopeString,"%f",slopes[0]);
                    sprintf(intString,"%f",intercepts[0]);
                    for(j=1; j < getAbsorbingWavelengthCount(i); ++j){
                        sprintf(slopeString,"%s,%f",slopeString,slopes[i]);
                        sprintf(intString,"%s,%f",intString,intercepts[i]);
                    }
                } else {
                    slopeString[0] = '\0';
                    intString[0] = '\0';
                }
                sqlite3_bind_text(pStmt,8,slopeString,strlen(slopeString),SQLITE_TRANSIENT);
                sqlite3_bind_text(pStmt,9,intString,strlen(intString),SQLITE_TRANSIENT);

                if(refSample){
                    sqlite3_bind_blob(pStmt,10,refSample->pixels,getNumPixels(i)*sizeof(float),SQLITE_TRANSIENT);
                } else {
                    sqlite3_bind_null(pStmt,10);
                }

                if (wavelengths){
                    sqlite3_bind_blob(pStmt,11,wavelengths,getNumPixels(i)*sizeof(double),SQLITE_TRANSIENT);
                } else {
                    sqlite3_bind_null(pStmt,11);
                }

                /*
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
                 */

                // Run Prepared Statement
                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to log to SQLite database for spectrometer %d.  Error: %s",i,sqlite3_errmsg(db));
                }

                // housekeeping!
                if(refSample) deallocateSample(&refSample);
                if(wavelengths) free(wavelengths);
                freeSpecConfig(&config);
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

uint64_t writeCTDData(struct CTDREADINGS ctd){
    uint64_t retVal = 0;
    char* insertStmt = "INSERT INTO ctd_readings (time,conductivity,temperature,depth,salinity) VALUES (?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;

    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,insertStmt,256,&pStmt,NULL) == SQLITE_OK){
            sqlite3_bind_int64(pStmt,1,(int64_t)ctd.t);
            sqlite3_bind_double(pStmt,2,(double)ctd.conductivity);
            sqlite3_bind_double(pStmt,3,(double)ctd.temperature);
            sqlite3_bind_double(pStmt,4,(double)ctd.pressure);
            sqlite3_bind_double(pStmt,5,(double)computeSalinity(ctd.conductivity,ctd.temperature,ctd.pressure));

            if(sqlite3_step(pStmt) == SQLITE_ERROR){
                syslog(LOG_DAEMON|LOG_ERR,"Unable to log CTD values to SQLite database.  Error: %s",sqlite3_errmsg(db));
            } else {
                retVal = sqlite3_last_insert_rowid(db);
            }

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

uint64_t writeCTDDefault(int specIndex){
    uint64_t retVal = 0;
    CTDreadings_s* ctd;

    ctd = getDwelledCTDValue(specIndex);
    if(ctd){
        retVal = writeCTDData(*ctd);
        free(ctd);
    }

    return retVal;
}

uint64_t writeWavelengthSet(time_t t, int64_t configID, float nonAbsWave, float nonAbsCount, float nonAbsAbsorbance, uint64_t ctd_id){
    uint64_t retVal=0;
    double heaterTemp = -1;

    heaterTemp = (double)getHeaterCurrentTemperature(1);

    const char* setStmt = "INSERT INTO main.wavelength_sets (time,config_id, non_absorbing_wavelength, original_count_non_absorbing, measured_absorbance_non_absorbing, ctd_reading_id, heater_temperature) VALUES (?,?,?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;

    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,setStmt,2048,&pStmt,NULL) == SQLITE_OK){
            sqlite3_bind_int64(pStmt,1,(int64_t)t);
            sqlite3_bind_int64(pStmt,2,configID);
            sqlite3_bind_double(pStmt,3,(double)nonAbsWave);
            sqlite3_bind_double(pStmt,4,(double)nonAbsCount);
            sqlite3_bind_double(pStmt,5,(double)nonAbsAbsorbance);
            sqlite3_bind_int64(pStmt,6,(int64_t)ctd_id);
            sqlite3_bind_double(pStmt,7,heaterTemp);
            if(sqlite3_step(pStmt) == SQLITE_ERROR){
                syslog(LOG_DAEMON|LOG_ERR, "Unable to log concentration set to SQLite database.  Error: %s",sqlite3_errmsg(db));
            } else {
                retVal = sqlite3_last_insert_rowid(db);
            }
            sqlite3_finalize(pStmt);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Cannot create prepared statement for writeWavelengthSet.  Error: %s", sqlite3_errmsg(db));
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Cannot log concentration set.  SQLite file is not open.");
    }
    pthread_mutex_unlock(&dataMutex);

    return retVal;
}

void writeConcData(unsigned char specID){
    uint64_t ctd_id = 0;   
    uint64_t set_id = 0;
    unsigned char waveCount = 0;
    time_t t;
    float* waves;
    float nonAbsWave;
    float* conc;
    float* abs;
    float* counts;

    t = time(NULL);

    waveCount = getAbsorbingWavelengthCount(specID);
    waves = getAbsorbingWavelengths(specID);
    if(!waves){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve wavelengths. Creating zero padded array.");
        waves = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!waves) {
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate waves array.");
            return;
        }
    }
    nonAbsWave = getNonAbsorbingWavelength(specID);

    conc = getConcentrations(specID);
    if(!conc){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve concentrations.  Creating zero padded array.");
        conc = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!conc) {
            free(waves);
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate concentration array.");
            return;
        }
    }

    abs = getAbsorbance(specID);
    if(!abs){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve absorbance.  Creating zero padded array.");
        abs = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!abs) {
            free(waves);
            free(conc);
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate absorbance array.");
            return;
        }
    }

    counts = getRawCounts(specID);
    if(!counts){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve concentrations.  Creating zero padded array.");
        counts = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!counts) {
            free(waves);
            free(conc);
            free(abs);
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate absorbance array.");
            return;
        }
    }

    // Log the current CTD
    ctd_id = writeCTDDefault(specID);

    // Log the concentration set
    set_id = writeWavelengthSet(t,currConfigID[specID],nonAbsWave,counts[MAX_ABS_WAVES],abs[MAX_ABS_WAVES],ctd_id); 

    pthread_mutex_lock(&dataMutex);
    if(db){
        const char* insertStmt = "INSERT INTO main.concentrations (wavelength_set_id,wavelength,concentration,original_count,measured_absorbance) VALUES (?,?,?,?,?)";
        sqlite3_stmt* pStmt = NULL;

        if(sqlite3_prepare(db,insertStmt,2048,&pStmt,NULL) == SQLITE_OK){
            unsigned int i;
            for(i=0; i < waveCount; ++i){
                sqlite3_bind_int64(pStmt,1,set_id);
                sqlite3_bind_double(pStmt,2,waves[i]);
                sqlite3_bind_double(pStmt,3,conc[i]);
                sqlite3_bind_double(pStmt,4,counts[i]);
                sqlite3_bind_double(pStmt,5,abs[i]);

                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to log concentration to DB for spectrometer #%d, wave #%d.  Error: %s",specID,i,sqlite3_errmsg(db));
                }

                // Reset Prepared Statement for Next Call
                sqlite3_reset(pStmt);
            }
        }
        sqlite3_finalize(pStmt);

    } else {
        syslog(LOG_DAEMON|LOG_ERR, "Unable to create prepared statement for concentrations data.  Error: %s",sqlite3_errmsg(db));
    }
    pthread_mutex_unlock(&dataMutex);

    //Housekeeping!
    free(waves);
    free(conc);
    free(abs);
    free(counts);
}

void writepHToDB(unsigned char specIndex, float pH,unsigned char absWaveCount, float* abs, struct CTDREADINGS ctd){
    uint64_t ctd_id, set_id;
    float* waves;
    float nonAbsWave;
    float* counts;
    time_t t;

    t = time(NULL);

    syslog(LOG_DAEMON|LOG_INFO,"Writing pH (%g) to SQLite DB for spectrometer %d.",pH,specIndex);
    waves = getAbsorbingWavelengths(specIndex);
    if(!waves){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve wavelengths. Creating zero padded array.");
        waves = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!waves) {
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate waves array.");
            return;
        }
    }
    nonAbsWave = getNonAbsorbingWavelength(specIndex);

    counts = getRawCounts(specIndex);
    if(!counts){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve concentrations.  Creating zero padded array.");
        counts = (float*)calloc(MAX_ABS_WAVES+1,sizeof(float));
        if(!counts) {
            free(waves);
            syslog(LOG_DAEMON|LOG_ERR,"SEVERE: Out of Memory.  Cannot allocate absorbance array.");
            return;
        }
    }

    // Log the CTD
    ctd_id = writeCTDData(ctd);

    // Log the wavelength set
    set_id = writeWavelengthSet(t,currConfigID[specIndex],nonAbsWave,counts[MAX_ABS_WAVES],abs[MAX_ABS_WAVES],ctd_id);

    pthread_mutex_lock(&dataMutex);
    if(db){
        // INSERT wavelengths
        const char* insertStmt = "INSERT INTO main.pH_wavelengths (wavelength_set_id,wavelength,original_count,measured_absorbance) VALUES (?,?,?,?)";
        sqlite3_stmt* pStmt = NULL;

        if(sqlite3_prepare(db,insertStmt,2048,&pStmt,NULL) == SQLITE_OK){
            unsigned int i;
            for(i=0; i < absWaveCount; ++i){
                sqlite3_bind_int64(pStmt,1,(int64_t)set_id);
                sqlite3_bind_double(pStmt,2,(double)waves[i]);
                sqlite3_bind_double(pStmt,3,(double)counts[i]);
                sqlite3_bind_double(pStmt,4,(double)abs[i]);

                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Unable to log pH wavelength to DB for spectrometer #%d, wave #%d.  Error: %s",specIndex,i,sqlite3_errmsg(db));
                }

                // Reset Prepared Statement for Next Call
                sqlite3_reset(pStmt);
            }
        }
        sqlite3_finalize(pStmt);

        // INSERT pH
        const char* pHinsertStmt = "INSERT INTO main.pH (wavelength_set_id,pH) VALUES (?,?)";
        sqlite3_stmt* pHStmt = NULL;

        if(sqlite3_prepare(db,pHinsertStmt,2048,&pHStmt,NULL) == SQLITE_OK){
            sqlite3_bind_int64(pHStmt,1,(int64_t)set_id);
            sqlite3_bind_double(pHStmt,2,(double)pH);

            if(sqlite3_step(pHStmt) == SQLITE_ERROR){
                syslog(LOG_DAEMON|LOG_ERR,"Unable to log pH to DB for spectrometer #%d.  Error: %s",specIndex,sqlite3_errmsg(db));
            }
        }
        sqlite3_finalize(pHStmt);

    } else {
        syslog(LOG_DAEMON|LOG_ERR, "Unable to create prepared statement for concentrations data.  Error: %s",sqlite3_errmsg(db));
    }
    pthread_mutex_unlock(&dataMutex);

    //Housekeeping!
    free(waves);
    free(counts);
}

void writeFullSpec(unsigned char specID){
    uint64_t ctd_id = 0;
    specSample* sample;

    char* insertStmt = "INSERT INTO full_spectrums (time,config_id,sample_spectrum,ctd_reading_id,heater_temperature) VALUES (?,?,?,?,?)";
    sqlite3_stmt* pStmt = NULL;

    ctd_id = writeCTDDefault(specID);

    pthread_mutex_lock(&dataMutex);
    if(db){
        if(sqlite3_prepare(db,insertStmt,1024,&pStmt,NULL) == SQLITE_OK){
            sample = getDefaultSample(specID);

            if(sample){
                sqlite3_bind_int64(pStmt,1,(uint64_t)time(NULL));
                sqlite3_bind_int64(pStmt,2,currConfigID[specID]);
                sqlite3_bind_blob(pStmt,3,sample->pixels,getNumPixels(specID)*sizeof(float),SQLITE_TRANSIENT);
                sqlite3_bind_int64(pStmt,4,ctd_id);
                sqlite3_bind_double(pStmt,5,(double)getHeaterCurrentTemperature(1));

                if(sqlite3_step(pStmt) == SQLITE_ERROR){
                    syslog(LOG_DAEMON|LOG_ERR,"Error logging full spectrum prepared statement for spectrometer #%d.  Error: %s",specID,sqlite3_errmsg(db));
                }
                deallocateSample(&sample);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve sample from spectrometer #%d.",specID);
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
    if(argc > 0){
        double* params = (double*)argv;
        unsigned char specID = (unsigned int)params[0];
        specID -= 1; // get to zero based index
        if(specID < NUM_SPECS){
            writeConcData(specID);
            syslog(LOG_DAEMON|LOG_INFO,"Concentration data written for spec %d", specID);
        } else {
            syslog(LOG_DAEMON|LOG_INFO,"Invalid spec ID for write concentrations command: %d.", specID);
        }
    }
}

void methodWriteFullSpec(unsigned long argc, void* argv){
    if(argc > 0){
        double* params = (double*)argv;
        unsigned char specID = (unsigned int)params[0];
        specID -= 1; // get to zero based index
        if(specID < NUM_SPECS){
            writeFullSpec(specID);
            syslog(LOG_DAEMON|LOG_INFO,"Full spectrum written for spec %d", specID);
        } else {
            syslog(LOG_DAEMON|LOG_INFO,"Invalid spec ID for write full spectrums command: %d", specID);
        }
    }
}


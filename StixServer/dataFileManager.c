static FILE* currDataFile=NULL;

// File Management Functions
void openDataFile(){
    char filePath[64];

    if(!currDataFile){
        sprintf(filePath,"%sDATA%u.csv",DATAFILEPATH,time(NULL));        
        syslog(LOG_DAEMON|LOG_INFO,"Opening data file @: %s.",filePath);
        currDataFile = fopen(filePath,"w+");
        if(!currDataFile){
            syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unable to open data file @: %s.",filePath);
        }
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

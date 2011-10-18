#include "benchConfig.h"

#define BENCHPATH "/etc/SEASServer/bench.txt"
#define MAXCONFIGLINE 128

static pthread_mutex_t benchConfigMutex = PTHREAD_MUTEX_INITIALIZER;
static BENCH_CONFIG_DATA benchConfig;

void initBenchConfig(){
    int i;

    pthread_mutex_lock(&benchConfigMutex);
    for(i=0; i < MAX_NUM_PUMPS; i++){
        benchConfig.PumpData[i].RPM = 0;
    }
    benchConfig.HeaterSetPointTemperature = 0;
    for(i=0; i < NUM_SPECS; i++){
        benchConfig.RangeData[i].TraceMinimum = -0.12;
        benchConfig.RangeData[i].TraceMaximum = 2;
        benchConfig.RangeData[i].SpectraMinimum = -1;
        benchConfig.RangeData[i].SpectraMaximum = 2;
    }
    pthread_mutex_unlock(&benchConfigMutex);
}

void syslogBenchConfig(const char* header, BENCH_CONFIG_DATA* config){
    int i;

    syslog(LOG_DAEMON|LOG_INFO,"%s",header);
    for(i=0; i < MAX_NUM_PUMPS; i++){
        syslog(LOG_DAEMON|LOG_INFO,"PUMP.%d=%d",i,config->PumpData[i].RPM);
    }
    syslog(LOG_DAEMON|LOG_INFO,"HEAT.%d=%f",0,config->HeaterSetPointTemperature);
    for(i=0; i < NUM_SPECS; i++){
        syslog(LOG_DAEMON|LOG_INFO,"ABSRANGE.%d=%f,%f",i,config->RangeData[i].TraceMinimum,config->RangeData[i].TraceMaximum);
    }
}

void writeBenchConfig(){
    int i;
    FILE* f = NULL;

    pthread_mutex_lock(&benchConfigMutex);
    syslog(LOG_DAEMON|LOG_INFO,"Writing Bench Config.");
    f = fopen(BENCHPATH,"w+");
    if(f){
        for(i=0; i < MAX_NUM_PUMPS; i++){
            fprintf(f,"PUMP.%d=%u\n",i,benchConfig.PumpData[i].RPM);
        }
        fprintf(f,"HEAT.%d=%f\n",0,benchConfig.HeaterSetPointTemperature);
        for(i=0; i < NUM_SPECS; i++){
            fprintf(f,"ABSRANGEMIN.%d=%f\n",i,benchConfig.RangeData[i].TraceMinimum);
            fprintf(f,"ABSRANGEMAX.%d=%f\n",i,benchConfig.RangeData[i].TraceMaximum);
            fprintf(f,"SPECTRARANGEMIN.%d=%f\n",i,benchConfig.RangeData[i].SpectraMinimum);
            fprintf(f,"SPECTRARANGEMAX.%d=%f\n",i,benchConfig.RangeData[i].SpectraMaximum);
        }
        fclose(f);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to write to bench config file at %s",BENCHPATH);
    }
    pthread_mutex_unlock(&benchConfigMutex);
}

void readBenchConfig(){
    FILE* f = NULL;
    char device[16];
    float value;
    char buf[1024];
    int devID = 0;
    struct stat s;

    device[0] = '\0';
    
    // If a bench configuration has never been created, initialize a default one and write it to a new file.
    if(stat(BENCHPATH,&s) == -1){
        syslog(LOG_DAEMON|LOG_INFO,"No file found at bench path.  Creating default file.");
        if(errno == ENOENT){
            initBenchConfig();
            writeBenchConfig();
            return;
        }
    }

    pthread_mutex_lock(&benchConfigMutex);
    syslog(LOG_DAEMON|LOG_INFO,"Reading Bench Config.");
    f = fopen(BENCHPATH,"r");
    if(f){
        while(!feof(f)){
            fgets(buf,1023,f);
            sscanf(buf,"%15[^.].%i=%f",device,&devID,&value);
            syslog(LOG_DAEMON|LOG_INFO,"Read the following line: %s.%i=%f",device,devID,value);
            if(strcmp(device,"PUMP") == 0){
                syslog(LOG_DAEMON|LOG_INFO,"Pump %d RPM Bench Config: %f",devID,value);
                benchConfig.PumpData[devID].RPM=(uint16_t)value;
            } else if(strcmp(device,"HEAT") == 0){
                benchConfig.HeaterSetPointTemperature = value;
            } else if(strcmp(device,"ABSRANGEMIN") == 0){
                benchConfig.RangeData[devID].TraceMinimum = value;
            } else if(strcmp(device,"ABSRANGEMAX") == 0){
                benchConfig.RangeData[devID].TraceMaximum = value;
            } else if(strcmp(device,"SPECTRARANGEMIN") == 0){
                benchConfig.RangeData[devID].SpectraMinimum = value;
            } else if(strcmp(device,"SPECTRARANGEMAX") == 0){
                benchConfig.RangeData[devID].SpectraMaximum = value;
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Did not recognize line in bench config: %s.%i=%f",device,devID,value);
            }
            device[0] = '\0';
        }
        fclose(f);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read bench config file at %s",BENCHPATH);
    }
    syslogBenchConfig("Read Config",&benchConfig);
    pthread_mutex_unlock(&benchConfigMutex);

    return;
}

void copyBenchConfig(BENCH_CONFIG_DATA* dest, BENCH_CONFIG_DATA* src){
    int i;

    for(i=0; i < MAX_NUM_PUMPS; i++){
        dest->PumpData[i].RPM = src->PumpData[i].RPM;
    }
    dest->HeaterSetPointTemperature = src->HeaterSetPointTemperature;
    for(i=0; i < NUM_SPECS; i++){
        dest->RangeData[i].TraceMinimum = src->RangeData[i].TraceMinimum;
        dest->RangeData[i].TraceMaximum = src->RangeData[i].TraceMaximum;
        dest->RangeData[i].SpectraMinimum = src->RangeData[i].SpectraMinimum;
        dest->RangeData[i].SpectraMaximum = src->RangeData[i].SpectraMaximum;
    }
}

void receiveSaveBenchConfig(int connection, char* command){
    SAVE_BENCHTOP_CONFIG_MSG msg;

    syslog(LOG_DAEMON|LOG_INFO,"Received Save Bench Config Command.");
    memcpy(&msg,command,sizeof(SAVE_BENCHTOP_CONFIG_MSG));

    pthread_mutex_lock(&benchConfigMutex);
    copyBenchConfig(&benchConfig,&(msg.BenchtopConfiguration));
    syslogBenchConfig("Bench Config",&benchConfig);
    syslogBenchConfig("Sent Config",&(msg.BenchtopConfiguration));
    pthread_mutex_unlock(&benchConfigMutex);

    writeBenchConfig();
    syslog(LOG_DAEMON|LOG_INFO,"Bench Config Saved.");
}

void sendBenchConfig(int connection, char* command){
    
    SAVE_BENCHTOP_CONFIG_MSG msg;
    
    syslog(LOG_DAEMON|LOG_INFO,"Sending Benchtop Config");
    msg.Command = RBC;

    pthread_mutex_lock(&benchConfigMutex);
    copyBenchConfig(&(msg.BenchtopConfiguration),&benchConfig);
    syslogBenchConfig("Sending Config",&(msg.BenchtopConfiguration));
    pthread_mutex_unlock(&benchConfigMutex);

    send(connection,&msg,sizeof(SAVE_BENCHTOP_CONFIG_MSG),0);
    syslog(LOG_DAEMON|LOG_INFO,"Benchtop Config Sent");
}

uint16_t getBenchPumpRPM(uint8_t pumpID){
    uint16_t RPM = 0;

    pthread_mutex_lock(&benchConfigMutex);
    RPM = benchConfig.PumpData[pumpID].RPM;
    pthread_mutex_unlock(&benchConfigMutex);

    return RPM;
}

float getBenchHeaterSetPoint(){
    float setPoint = 0;

    pthread_mutex_lock(&benchConfigMutex);
    setPoint = benchConfig.HeaterSetPointTemperature;
    pthread_mutex_unlock(&benchConfigMutex);

    return setPoint;
}

float getBenchAbsRangeMin(uint8_t specID){
    float absMin = 0;

    pthread_mutex_lock(&benchConfigMutex);
    absMin = benchConfig.RangeData[specID].TraceMinimum;
    pthread_mutex_unlock(&benchConfigMutex);
    
    return absMin;
}

float getBenchAbsRangeMax(uint8_t specID){
    float absMax = 0;
    
    pthread_mutex_lock(&benchConfigMutex);
    absMax = benchConfig.RangeData[specID].TraceMaximum;
    pthread_mutex_unlock(&benchConfigMutex);

    return absMax;
}

float getBenchSpectraRangeMin(uint8_t specID){
    float spectraMin = 0;

    pthread_mutex_lock(&benchConfigMutex);
    spectraMin = benchConfig.RangeData[specID].SpectraMinimum;
    pthread_mutex_unlock(&benchConfigMutex);

    return spectraMin;
}

float getBenchSpectraRangeMax(uint8_t specID){
    float spectraMax = 0;

    pthread_mutex_lock(&benchConfigMutex);
    spectraMax = benchConfig.RangeData[specID].SpectraMaximum;
    pthread_mutex_unlock(&benchConfigMutex);

    return spectraMax;
}


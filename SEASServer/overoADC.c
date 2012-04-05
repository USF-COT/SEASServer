
#include "overoADC.h"

/* OVEROADC.H - Used only to read the battery voltage from TWL-4030 Channel 3.
 * 
 * By: Michael Lindemuth
 *  
 * Based on documentation at: http://www.sakoman.com/cgi-bin/gitweb.cgi?p=linux-omap-2.6.git;a=blob;f=Documentation/hwmon/twl4030-madc-hwmon;h=ef7984317cecb29eca6ef23a30c21d920e66ef31;hb=refs/heads/omap-2.6.39-pm 
*/

#define BATT_ADC_BUFSIZE 8

static pthread_mutex_t batteryMutex = PTHREAD_MUTEX_INITIALIZER;

float getBatteryVoltage(){
    FILE* hwmon = NULL;
    char buffer[BATT_ADC_BUFSIZE];
    float retVal = -1;
    buffer[0] = '0';
    buffer[0] = '\0';

    pthread_mutex_lock(&batteryMutex);
    hwmon = fopen("/sys/class/hwmon/hwmon0/device/in3_input","r");
    if(hwmon){
        fgets(buffer,BATT_ADC_BUFSIZE,hwmon);
        fclose(hwmon);

        // Process voltage reading
        float rawVoltage = atof(buffer)/(float)1000; // Convert int mV to raw V
        if(rawVoltage != 0){
            retVal = rawVoltage * 12;
        } 
    } else {
        syslog(LOG_DAEMON|LOG_INFO,"Unable to open hardware monitor.");
    }    
    pthread_mutex_unlock(&batteryMutex);
    return retVal; 
}

void receiveGetBatteryVoltage(int connection, char* command){
    unsigned char sendBuffer[5];
    float batteryVoltage;

    if(command[0] == RBS){
        syslog(LOG_DAEMON|LOG_INFO,"Getting battery voltage for GUI");
        batteryVoltage = getBatteryVoltage();
        syslog(LOG_DAEMON|LOG_INFO,"Retrieved battery voltage as: %f.",batteryVoltage);
        sendBuffer[0] = RBS;
        memcpy(sendBuffer+1,&batteryVoltage,4);
        send(connection,sendBuffer,5,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveBatteryVoltage method.");
    }
}


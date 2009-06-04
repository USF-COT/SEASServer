#include "config.h"
#include "requestdispatch.h"
#include <stdio.h>
#include <unistd.h>

char readConfig(char* path){

    FILE* configFile;
    char configLine[MAXCONFIGLINE];
    char configPrefix[MAXCONFIGPREFIX];
    char configSuffix[MAXCONFIGSUFFIX];
    char request[MAXREQUEST];

    if(access(path,R_OK||W_OK)){
        return 0; // Do not have access to file
    }

    configFile = fopen(path,"r+");
    while(!feof(configFile)){
        fgets(configLine,MAXCONFIGLINE,configFile);
        sscanf(configLine,"%s=%s",configPrefix,configSuffix);

        if(strncmp("PUMPCTRL",configPrefix,MAXCONFIGPREFIX) == 0){
            if(strncmp("ON",configSuffix,MAXCONFIGSUFFIX) == 0){
                
            }
        }
    }
    return 1;
}

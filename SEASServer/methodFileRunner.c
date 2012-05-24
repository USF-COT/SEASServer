#include "methodFileRunner.h"
#include "dataFileManager.h"
#include "methodFileManager.h"

volatile sig_atomic_t executingMethod = 0;
static pthread_t methodFileRunnerThread;

/*
void *methodFileRunner(void* name){
    char* filename = (char*)getActiveMethodFilename();

    yyin = fopen(filename,"r");
    if(yyin){
        runNodes();
    }

    syslog(LOG_DAEMON|LOG_INFO,"Running Method File: %s.",filename);
    executingMethod = 1;
    while(executingMethod){
        sleep(1);
    }
    stopNodes();
    clearNodes();
    syslog(LOG_DAEMON|LOG_INFO,"Stopped Running Method File: %s.",filename);
    free(name);
    pthread_exit(NULL);
}
*/

void *methodFileRunner(void* name){
    receiveExecuteMethod(-1,NULL);
}

// Generic Methods
void executeMethodFile(){
    pthread_attr_t methodFileAttr;

    if(!executingMethod){
        pthread_attr_init(&methodFileAttr);
        pthread_attr_setdetachstate(&methodFileAttr, PTHREAD_CREATE_DETACHED);
        pthread_create(&methodFileRunnerThread,&methodFileAttr,methodFileRunner,NULL);
    }
}

void terminateMethodFile(){
    executingMethod = 0;
}

void terminateMethodAndWait(){
    executingMethod = 0;
    pthread_join(methodFileRunnerThread,NULL);
}

void waitOnMethodFile(){
    if(executingMethod)
        pthread_join(methodFileRunnerThread,NULL);
}

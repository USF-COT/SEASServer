#include "methodFileRunner.h"

volatile sig_atomic_t executingMethod = 0;
pthread_t methodFileRunnerThread;

void *methodFileRunner(void* name){
    char* filename = (char*)name;

    executingMethod = 1;
    while(executingMethod){
     
    }
    free(filename);
}

// Generic Methods
void executeMethodFile(){
    pthread_attr_t methodFileAttr;

    if(!executingMethodFile){
        pthread_attr_init(&executingMethodFile);
        pthread_attr_setdetachstate(&methodFileAttr, PTHREAD_CREATE_DETACHED);
        pthread_create(&methodFileRunnerThread,&methodFileAttr,methodFileRunner,(void *)getActiveMethodFilename());
    }
}

void terminateMethodFile(){
    executingMethod = 0;
}

// GUI specific wrapper methods
void receiveExecuteMethod(int connection, char* command){
    executeMethodFile();
}
void receiveTerminateMethod(int connection, char* command){
    terminateMethodFile();
}

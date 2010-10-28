#include "methodFileRunner.h"

volatile sig_atomic_t executingMethod = 0;
static pthread_t methodFileRunnerThread;

void *methodFileRunner(void* name){
    struct s_node* node;
    char* filename = (char*)name;

    yyin = fopen("test.m","r");
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
    free(filename);
}

// Generic Methods
void executeMethodFile(){
    pthread_attr_t methodFileAttr;

    if(!executingMethod){
        pthread_attr_init(&methodFileAttr);
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

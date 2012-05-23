#include "methodFileManager.h"

#define MAXMETHODPATHLENGTH 512
#define STORAGEDIRECTORY "/home/datauser/methods/"
#define ACTIVESCRIPTCONFIGFILE ".active.config"
#define MAXMETHRECVBUF 512

int checkStorageDirectory(){
    struct stat statResult;

    if(stat(STORAGEDIRECTORY,&statResult) == -1){
       if(errno == ENOENT){
           syslog(LOG_DAEMON|LOG_INFO,"Creating Directory: %s",STORAGEDIRECTORY);
           if(mkdir(STORAGEDIRECTORY,S_IRWXU|S_IRWXG|S_IRWXO) == -1){
               syslog(LOG_DAEMON|LOG_INFO,"Error Creating Directory");
               return -1;
           }
       }
    }
    return 1;
}

FILE* createMethodFile(char* filename){
    char fullPath[MAXMETHODPATHLENGTH];
    unsigned short length = 0;
    FILE* methodFile = NULL;

    length += strlen(STORAGEDIRECTORY);
    if(length < MAXMETHODPATHLENGTH){
        sprintf(fullPath,"%s",STORAGEDIRECTORY);
        length += strlen(filename)+1;
        // Check if the path is ok
        if(checkStorageDirectory() == -1){
            return NULL;
        }

        if(length < MAXMETHODPATHLENGTH){
            strcat(fullPath,filename);
            syslog(LOG_DAEMON|LOG_INFO,"Opening File: %s",fullPath);
            methodFile = fopen(fullPath,"w+");
            if(methodFile == NULL){
                syslog(LOG_DAEMON|LOG_INFO,"Error Creating File");
            }
        }
    }

    return methodFile;
}

void receiveMethodFile(int connection, char* command){
    // Parse filename from first line
    char *filename = NULL;
    char defaultFilename[48] = {'\0'}; 
    char response[1] = {ACK};
    FILE* methodFile = NULL;
    
    char receiveBuffer[MAXMETHRECVBUF+1];

    int i,offset = 0,nBytesReceived = 0;

    syslog(LOG_DAEMON|LOG_INFO,"Opening method file.");
    filename = strtok(command+1,"\n");
    if(filename){
        methodFile = createMethodFile(filename);
        offset = strlen(filename+1);
    } else {
        sprintf(defaultFilename,"%u.m",(unsigned int)time(NULL));
        methodFile = createMethodFile(defaultFilename);
    }

    syslog(LOG_DAEMON|LOG_INFO,"Storing contents in method file: %s", filename);
    // If a method file exists, output the remaining characters in the buffer to the file.
    // When an LRM character is found, close the file and return an LRM as a response.
    if(methodFile){
        // Write out remainder of command buffer
        for(i=offset; i < strlen(command); i++){
            if(command[i] == LRM){
                // Close the file
                fclose(methodFile);
                methodFile = NULL;

                // Create a response
                send(connection,(void*)response,1,0);
                syslog(LOG_DAEMON|LOG_INFO,"Successfully stored method file %s", filename);
                return;
            } else {
                fputc(command[i],methodFile);
            } 
        }

        // Retrieve the rest of the file stream from the socket
        send(connection,(void*)response,1,0);
        nBytesReceived = recv(connection,receiveBuffer,MAXMETHRECVBUF,0);
        while(nBytesReceived > 0){
            receiveBuffer[nBytesReceived] = '\0';
            for(i=0; i < strlen(receiveBuffer); i++){
                if(receiveBuffer[i] == LRM){
                    // Close the file
                    fclose(methodFile);
                    methodFile = NULL;

                    // Create a response
                    send(connection,(void*)response,1,0);
                    syslog(LOG_DAEMON|LOG_INFO,"Successfully stored method file %s", filename);
                    return;
                } else {
                    fputc(receiveBuffer[i],methodFile);
                }
            }
            send(connection,(void*)response,1,0);
            nBytesReceived = recv(connection,receiveBuffer,MAXMETHRECVBUF,0);
        }
    } else {  // If a file was never created, return an error code.  This one is just a placeholder.
        syslog(LOG_DAEMON|LOG_ERR,"Never was able to create method file names: %s", filename);
    }

}

int filterHidden(const struct dirent *entry){
    if(entry->d_name[0] == '.'){
        return 0;
    } else {
        return 1;
    }
}

void sendMethodFileList(int connection, char* command){
    struct dirent **namelist;
    int n;
    char response[1] = {LMT};

    syslog(LOG_DAEMON|LOG_INFO,"Sending method file list...");
    n = scandir(STORAGEDIRECTORY, &namelist, filterHidden, alphasort);
    if(n < 0){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to scan storage directory.");
    } else {
        while(n--){
            send(connection,(void*)namelist[n]->d_name,strlen(namelist[n]->d_name),0);
            send(connection,(void*)"\n",1,0);
            free(namelist[n]);
        }
        free(namelist);
    }
    send(connection,(void*)response,1,0);
    syslog(LOG_DAEMON|LOG_INFO,"Method file list sent.");
}

char* createFullPath(char* filename){
    char* fullPath;

    fullPath = malloc(sizeof(char)*(strlen(STORAGEDIRECTORY)+strlen(filename)+1));
    fullPath[0] = '\0';
    strcat(fullPath,STORAGEDIRECTORY);
    strcat(fullPath,filename);
    return fullPath;
}

void sendMethodFile(int connection, char* command){
    char* filename;
    char* fullPath;
    
    FILE* fileStream;
    char contentsBuffer[MAXMETHRECVBUF+1];
    int nBytes;

    char response[1] = {RMF};

    filename = strtok(command+1,"\n");
    syslog(LOG_DAEMON|LOG_INFO,"Read filename as %s",filename);

    syslog(LOG_DAEMON|LOG_INFO,"Sending file.");
    if(checkStorageDirectory() != -1){
        fullPath = createFullPath(filename);
        syslog(LOG_DAEMON|LOG_INFO,"Reading lines in file at path: %s",fullPath);
        fileStream = fopen(fullPath,"r");
        if(fileStream){
            nBytes = fread(contentsBuffer,sizeof(char),MAXMETHRECVBUF,fileStream);
            while(nBytes > 0){
                send(connection,(void*)contentsBuffer,nBytes,0);
                nBytes = fread(contentsBuffer,sizeof(char),MAXMETHRECVBUF,fileStream);
            }
            fclose(fileStream);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Error reading data from file: %s.  Returning empty string.", filename);
        }
        free(fullPath);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to find or create method script storage directory.");
    }
    
    send(connection,(void*)response,1,0);    
    syslog(LOG_DAEMON|LOG_INFO,"File sent.");
}

void receiveSetCurrentMethodFile(int connection, char* command){
    char* filename;
    char* activeConfigPath;

    FILE* activeConfigFile;

    filename = strtok(command+1,"\n");
    syslog(LOG_DAEMON|LOG_INFO,"Setting %s as current method file.",filename);
    
    if(checkStorageDirectory() != -1){
        activeConfigPath = createFullPath(ACTIVESCRIPTCONFIGFILE);
        activeConfigFile = fopen(activeConfigPath,"w+");
        if(activeConfigFile){
            if(fwrite(filename,sizeof(char),strlen(filename),activeConfigFile) < strlen(filename)){
                syslog(LOG_DAEMON|LOG_ERR,"Not all bytes were written to active config file @: %s.  Check file to see what was cut off.", activeConfigPath);
            }
            fclose(activeConfigFile); 
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to open active script configuration file @: %s",activeConfigPath);
        } 
        free(activeConfigPath);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to find or create method script storage directory.");
    }
}

void receiveDeleteMethodFile(int connection, char* command){
    char* filename;
    char* fullPath;

    int removeStatus = -1;

    filename = strtok(command+1,"\n");
    syslog(LOG_DAEMON|LOG_INFO,"Deleting file: %s", filename);

    if(checkStorageDirectory() != -1){
        fullPath = createFullPath(filename);
        removeStatus = remove(fullPath);
        if(removeStatus == 0){
            syslog(LOG_DAEMON|LOG_INFO,"Deleted file @: %s",fullPath);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to remove file @: %s",fullPath);
        }
        free(fullPath);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to find or create method script storage directory.");
    }
}

char* getActiveMethodFilename(){
    int bytesRead;
    char* fullPath;
    char* filename = NULL;

    FILE* activeConfigFile;

    if(checkStorageDirectory() != -1){
        fullPath = createFullPath(ACTIVESCRIPTCONFIGFILE);
        activeConfigFile = fopen(fullPath,"r");
        if(activeConfigFile){
            filename = (char *)calloc(257,sizeof(char)); // 256 is the max size of a filename on a standard fs + 1 for the null terminator
            if((bytesRead = fread(filename,sizeof(char),256,activeConfigFile)) == 0){
                if(ferror(activeConfigFile)){
                    free(filename);
                    filename = NULL;
                }
            }
            filename[bytesRead] = '\0'; // Clean up the EOF character at the end.
            fclose(activeConfigFile);
        }
        free(fullPath);
    }
    return filename;
}

void sendActiveMethodFile(int connection, char* command){
    char* filename;
    char response[258]; // 256 for filename + 1 for command + 1 for terminal
    char commandTerminal[2] = {'\n','\0'};

    // Initialize response
    response[0] = '\0';

    syslog(LOG_DAEMON|LOG_INFO,"Sending active filename.");

    filename = getActiveMethodFilename();
    if(filename){
        if(strlen(filename) == 0){
            syslog(LOG_DAEMON|LOG_INFO,"No method file active.  Sending empty string.");
        }
        strcat(response,filename);
        free(filename);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to read filename from active config file. Returning empty string.");
    }   

    strcat(response,commandTerminal);
    send(connection,(void*)response,strlen(response),0);
    syslog(LOG_DAEMON|LOG_INFO,"Sent active filename: %s",response);
}

void receiveExecuteMethod(int connection, char* command){
    s_node* node = NULL;
    s_node* nextNode = NULL;
    char* filename = NULL;
    char* fullPath = NULL;
    int numBytesReceived = 0;
    char termRecBuf[2] = {'\0','\0'};
    RUNTIME_RESPONSE_HEADER termResponse;

    termResponse.HeadByte = RTH;
    termResponse.Command = METHOD_COMPLETED_CMD;
    
    filename = getActiveMethodFilename();
    if(filename){
        fullPath = createFullPath(filename);
        if(fullPath){
            syslog(LOG_DAEMON|LOG_INFO,"Executing method file @ path: %s.",fullPath);
            yyin = fopen(fullPath,"r");
            if(yyin){
                clearNodes();
                yyparse();
                fclose(yyin);
                node = getHeadNode();
                setSocketTimeout(connection,1,0);
                syslog(LOG_DAEMON|LOG_INFO,"Sucessfully started executing method file @ path: %s.",fullPath);
                while(node != NULL){
                    nextNode = evaluateNode(node);
                    sendRunProtocolMessage(connection,node);
                    node = nextNode;
                    numBytesReceived = recv(connection,termRecBuf,1,0);
                    if(numBytesReceived > 0 && termRecBuf[0] == TRM){
                        syslog(LOG_DAEMON|LOG_INFO,"Terminate Command Received.  Stopping Node Execution.");
                        break;
                    } 
                }
                setSocketTimeout(connection,0,0);
                syslog(LOG_DAEMON|LOG_INFO,"Execution finished for file @ path: %s.",fullPath);
            } else {
                syslog(LOG_DAEMON|LOG_ERR,"Unable to open method file @: %s.",fullPath);
            }
            free(fullPath);
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve method full path.");
        }
        free(filename);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to retrieve method filename.");
    }
    
    // Let the GUI know that method file execution has terminated
    send(connection,(void*)&termResponse,sizeof(RUNTIME_RESPONSE_HEADER),0);
}

void receiveTerminateMethod(int connection, char* command){
    syslog(LOG_DAEMON|LOG_INFO,"Terminating method file execution.");
    stopNodes();
    clearNodes();
    syslog(LOG_DAEMON|LOG_INFO,"Method file execution terminated.");
}

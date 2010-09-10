#include "methodFileManager.h"

#define MAXMETHODPATHLENGTH 512
#define STORAGEDIRECTORY "/media/card/methods/"
#define ACTIVESCRIPTCONFIGFILE ".active.config"

static FILE* methodFile;
static char* methodBuffer;

int checkStorageDirectory(){
    struct stat* statResult;

    if(stat(STORAGEDIRECTORY,statResult) == -1){
       if(errno == ENOENT){
           syslog(LOG_DAEMON||LOG_INFO,"Creating Directory: %s",STORAGEDIRECTORY);
           if(mkdir(STORAGEDIRECTORY,S_IRWXU|S_IRWXG|S_IRWXO) == -1){
               syslog(LOG_DAEMON||LOG_INFO,"Error Creating Directory");
               return -1;
           }
       }
    }
    return 1;
}

int createMethodFile(char* filename){
    struct stat statResult;
    char fullPath[MAXMETHODPATHLENGTH];
    unsigned short length = 0;

    length += strlen(STORAGEDIRECTORY);
    if(length < MAXMETHODPATHLENGTH){
        sprintf(fullPath,"%s",STORAGEDIRECTORY);
        length += strlen(filename)+1;
        // Check if the path is ok
        if(checkStorageDirectory() == -1){
            return -1;
        }

        if(length < MAXMETHODPATHLENGTH){
            strcat(fullPath,filename);
            syslog(LOG_DAEMON||LOG_INFO,"Opening File: %s",fullPath);
            methodFile = fopen(fullPath,"w+");
            if(methodFile == NULL){
                syslog(LOG_DAEMON||LOG_INFO,"Error Creating File");
                return -1;
            }
        }
    } else {
        return -1;
    }
    return 0;
}

GUIresponse* receiveMethodFile(char* contentBuffer){
    // Parse filename from first line
    char *tokenBuffer = NULL;
    char defaultFilename[24] = {'\0'}; 
    char endOfTransChar[2] = {LRM,'\0'};
    GUIresponse* response = NULL;

    int i,offset = 0;

    // If no method file is initialized yet, create it according to a specified file name.
    // Set an offset so that the filename is not store in the file
    if(!methodFile){
        tokenBuffer = strtok(contentBuffer,"\n");
        if(tokenBuffer){
            createMethodFile(tokenBuffer+1);
            offset = strlen(tokenBuffer);
        } else {
            sprintf(defaultFilename,"%u",time(NULL));
            createMethodFile(defaultFilename);
        }
    }

    // If a method file exists, output the remaining characters in the buffer to the file.
    // When an LRM character is found, close the file and return an LRM as a response.
    if(methodFile){
        for(i=0+offset; i < strlen(contentBuffer); i++){
            if(contentBuffer[i] == LRM){
                // Close the file
                fclose(methodFile);
                methodFile = NULL;

                // Create a response
                response = malloc(sizeof(GUIresponse));
                response->length = 1;
                response->response = malloc(sizeof(char));
                ((char*)response->response)[0] = LRM;
                return response;
            } else {
                fputc(contentBuffer[i],methodFile);
            }
        }
    } else {  // If a file was never created, return an error code.  This one is just a placeholder.
        return createResponse(6,"Error");
    }

    return response;
}

int filterHidden(const struct dirent *entry){
    if(entry->d_name[0] == '.'){
        return 0;
    } else {
        return 1;
    }
}

GUIresponse* getMethodFileList(){
    struct dirent **namelist;
    int n;

    char* nlDelimFilenames;
    GUIresponse* response;    

    n = scandir(STORAGEDIRECTORY, &namelist, filterHidden, alphasort);
    if(n < 0){
        syslog(LOG_DAEMON||LOG_ERR,"Unable to scan storage directory.");
        nlDelimFilenames = malloc(sizeof(char)*2);
        nlDelimFilenames[0] = LMT;
        nlDelimFilenames[1] = '\0';
    } else {
        // Max filename length in dirent structure is 256, so plan on that plus a newline for each entry the + 2 is for the LMT end of stream token and '\0'
        nlDelimFilenames = malloc(sizeof(char)*(257*n+2));
        nlDelimFilenames[0] = '\0';
        while(n--){
            strcat(nlDelimFilenames,namelist[n]->d_name);
            strcat(nlDelimFilenames,"\n");
            free(namelist[n]);
        }
        free(namelist);

        // Reuse n as an index to add the LMT char and null terminator '\0'
        n = strlen(nlDelimFilenames);
        nlDelimFilenames[n] = LMT;
        nlDelimFilenames[n+1] = '\0';
    }
    response = createResponseString(nlDelimFilenames);
    free(nlDelimFilenames);
    return response;
}

GUIresponse* readMethodFile(char* contentBuffer){
    char* fileContents = NULL;
    char* filename;
    char* fullPath;
    int fd, nBytes;
    struct stat *fileStats;
    GUIresponse* response;

    filename = strtok(contentBuffer+1,"\n");
    syslog(LOG_DAEMON||LOG_INFO,"Read filename as %s",filename);

    //if(checkStorageDirectory() != -1){
        syslog(LOG_DAEMON||LOG_INFO,"Allocating memory.");
        fullPath = (char *)malloc(sizeof(char)*(strlen(STORAGEDIRECTORY)+strlen(filename)+1));
        syslog(LOG_DAEMON||LOG_INFO,"Memory allocated.");
        fullPath[0] = '\0';
        strcat(fullPath,STORAGEDIRECTORY);
        strcat(fullPath,filename);
        syslog(LOG_DAEMON||LOG_INFO,"Reading lines in file at path: %s",fullPath);
        fd = open(fullPath,O_RDONLY);
        if(fd != -1){
          if(fstat(fd,fileStats) == 0){
              fileContents = malloc(sizeof(char) * (fileStats->st_size+2));
              if(read(fd,(void*)fileContents,fileStats->st_size) != -1){
                  fileContents[fileStats->st_size] = RMF;
                  fileContents[fileStats->st_size] = '\0';
               } else {
                   free(fileContents);
                   fileContents = NULL;
               }
           }
           close(fd);
        }
        free(fullPath);
    //}
    
    if(fileContents == NULL){
        syslog(LOG_DAEMON||LOG_ERR,"Error reading data from file: %s.  Returning empty string.", filename);
        fileContents = malloc(sizeof(char)*2);
        fileContents[0] = RMF;
        fileContents[1] = '\0';
    } 
    
    response = createResponseString(fileContents);
    free(fileContents);
    return response;        
}

GUIresponse* setActiveMethod(char* contentBuffer){
}


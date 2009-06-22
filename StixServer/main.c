/* 
 * File:   main.c
 * Author: Michael Lindemuth
 *
 * Main does the following:
 * 1) Starts the StixServer daemon process
 * 2) Opens daemon syslog and logs errors to it
 * 3) Start TCP/IP socket listener
 * 4) Spawns up to 10 request handling threads when users connect to TCP/IP socket
 *
 * Daemon code derived from: http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 * Socket code derived from: http://www.paulgriffiths.net/program/c/srcs/echoservsrc.html
 *
 * Created on May 29, 2009, 2:40 PM
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include "parseGUI.h"

#define CONFIGPATH "/media/card/StixServerConfig.txt"
#define NUM_THREADS 10
#define MAXBUF 512
#define DEBUG 1

typedef enum {UNAVAILABLE,AVAILABLE}AVAILABILITY;

pthread_t thread_bin[NUM_THREADS];
AVAILABILITY thread_bin_available[NUM_THREADS];

typedef struct THREADINFO{
    unsigned short thread_bin_index;
    int socket_connection;
}threadInfo;

void* handleConnection(void* info){
    int* connection = &((threadInfo*)info)->socket_connection;
    char buffer[MAXBUF+1];
    char hexString[3*MAXBUF+10];
    char hexBuf[4];
    GUIresponse* response;
    int i,j;
    int numBytesReceived;    

    numBytesReceived = recv(*connection,buffer,MAXBUF,0);
    while(numBytesReceived > 0){
#ifdef DEBUG
	hexString[0] = '\0';
	hexBuf[0] = '\0';
        for(i=0; i<numBytesReceived;i++){
		snprintf(hexBuf,4,"%02X ",buffer[i]);
		strncat(hexString,hexBuf,4);	
	}
        syslog(LOG_DAEMON||LOG_INFO,"(Thread %i)Received(%i bytes): %s",((threadInfo*)info)->thread_bin_index,numBytesReceived,hexString);
#endif

	response = parseGUI(buffer);
        if(response){
#ifdef DEBUG
            hexString[0] = '\0';
            hexBuf[0] = '\0';
            for(i=0; i < MAXBUF*3 && i < response->length; i++){
                snprintf(hexBuf,4,"%02X ",((char*)response->response)[i]);
                strncat(hexString,hexBuf,4);
            }
            syslog(LOG_DAEMON||LOG_INFO,"(Thread %i)Sending(%i bytes): %s",((threadInfo*)info)->thread_bin_index,i,hexString);
#endif
            send(*connection,response->response,response->length,0);
            freeResponse(response);
        }
        else
            syslog(LOG_DAEMON||LOG_ERR,"Unknown command sent.  Unable to handle request.  Continuing...");

        numBytesReceived = recv(*connection,buffer,MAXBUF,0);
    }

    if ( close(*connection) < 0 ) {
        syslog(LOG_DAEMON||LOG_ERR,"(Thread %i)Error calling close() on connection socket. Daemon Terminated.",((threadInfo*)info)->thread_bin_index);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DAEMON||LOG_INFO,"(Thread %i)Connected Socket Closed.",((threadInfo*)info)->thread_bin_index);
    thread_bin_available[((threadInfo*)info)->thread_bin_index] = AVAILABLE;
    free(info);
    pthread_exit(NULL);
}

int main(){
  pid_t pid, sid;
  int       list_s;                /*  listening socket          */
  int       conn_s;                /*  connection socket         */
  short int port = 1995;                  /*  port number               */
  struct    sockaddr_in servaddr;  /*  socket address structure  */
  int i,availableThreadID;
  threadInfo* info;

  /* Fork off the parent process */
  pid = fork();
  if(pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* If we got a good PID, then we can exit the parent process */
  if(pid > 0){
    exit(EXIT_SUCCESS);
  }

  umask(0);

  printf("Daemon started.  Writing all further notices to daemon log: /var/log/daemon.log\n");

  /* Open Log File Here */
  openlog("STIXSERVER",LOG_PID,LOG_DAEMON);
  syslog(LOG_DAEMON||LOG_INFO,"Daemon Started.");

  /* Open Config File Here */
  /*
  if(!readConfig(CONFIGPATH)){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to Read Configuration File.  Daemon Terminated.\n");
    exit(EXIT_FAILURE);
  }
  */

  /* Create a new SID for the child process */
  sid = setsid();
  if(sid < 0){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to create a new SID for child process. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if((chdir("/")) < 0){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to switch working directory. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* Setup TCP/IP Socket */
  if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      syslog(LOG_DAEMON||LOG_ERR,"Unable to create socket. Daemon Terminated.");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  // Connect the USB Spectrometers
  connectSpectrometers("USB4F02572","blah");

  /*  Bind our socket addresss to the
	listening socket, and call listen()  */

  if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    syslog(LOG_DAEMON||LOG_ERR,"Unable to bind socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  if ( listen(list_s, NUM_THREADS) < 0 ) {
    syslog(LOG_DAEMON||LOG_ERR,"Unable to listen on socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  for(i=0; i < NUM_THREADS; i++)
      thread_bin_available[i] = AVAILABLE;

  while(1){
    syslog(LOG_DAEMON||LOG_INFO,"Listening for connection on port %i", port);
    /* Wait for TCP/IP Connection */
    if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
        syslog(LOG_DAEMON||LOG_ERR,"Unable to call accept() on socket. Daemon Terminated.");
        exit(EXIT_FAILURE);
    }

    /* Spawn a POSIX Server Thread to Handle Connected Socket */
    for(i=0; i < NUM_THREADS; i++){
        if(thread_bin_available[i]){
            thread_bin_available[i] = UNAVAILABLE;
            syslog(LOG_DAEMON||LOG_INFO,"Handling new connection on port %i",port);
            info = malloc(sizeof(threadInfo));
            info->socket_connection = conn_s;
            info->thread_bin_index = i;
            pthread_create(&thread_bin[i],NULL,handleConnection, (void*)info);
            break;
        }
    }

    if(i > NUM_THREADS){
        syslog(LOG_DAEMON||LOG_ERR,"Unable to create thread to handle connection.  Continuing...");
    }
    
  }
  
  // Disconnect Spectrometers
  disconnectSpectrometers();
  syslog(LOG_DAEMON||LOG_INFO,"Daemon Exited.");
  exit(EXIT_SUCCESS);

}


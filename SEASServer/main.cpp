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

#define DEBUG 1

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
#include <signal.h>
#include "benchConfig.h"
#include "config.h"
#include "parseGUI.h"
#include "USB4000Manager.h"
#include "LONDispatch.h"
#include "SEASPeripheralCommands.h"
#include "dataFileManager.h"
#include "CTDSink.h"

#define LONPORT "/dev/ttyO0"

#define NUM_THREADS 10
#define MAXBUF 512

typedef enum {UNAVAILABLE,AVAILABLE}AVAILABILITY;

pthread_t thread_bin[NUM_THREADS];
AVAILABILITY thread_bin_available[NUM_THREADS];

typedef struct THREADINFO{
    unsigned short thread_bin_index;
    int socket_connection;
}threadInfo;

volatile sig_atomic_t keep_going = 1;
volatile sig_atomic_t numClients = 0;
int list_s = 0;

void catch_term(int sig)
{
    syslog(LOG_DAEMON|LOG_INFO,"SIGTERM Caught.");
    keep_going = 0;
    closeDataFile();
    close(list_s);
    stopDispatch();
}

void* handleConnection(void* info){
    int* connection = &((threadInfo*)info)->socket_connection;
    char buffer[MAXBUF+1];
    char hexString[3*MAXBUF+10];
    char hexBuf[4];
    int i;
    int numBytesReceived;    

    numBytesReceived = recv(*connection,buffer,MAXBUF,0);
    while(numBytesReceived > 0 && keep_going){

        // Terminate buffer
        buffer[numBytesReceived] = '\0';
#ifdef DEBUG
        hexString[0] = '\0';
	hexBuf[0] = '\0';
        for(i=0; i<numBytesReceived;i++){
		snprintf(hexBuf,4,"%02X ",buffer[i]);
		strncat(hexString,hexBuf,4);	
	}
        syslog(LOG_DAEMON|LOG_INFO,"(Thread %i)Received(%i bytes): %s",((threadInfo*)info)->thread_bin_index,numBytesReceived,hexString);
#endif

        parseGUI(*connection,buffer);
        numBytesReceived = recv(*connection,buffer,MAXBUF,0);
    }

    if ( close(*connection) < 0 ) {
        syslog(LOG_DAEMON|LOG_ERR,"(Thread %i)Error calling close() on connection socket. Daemon Terminated.",((threadInfo*)info)->thread_bin_index);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DAEMON|LOG_INFO,"(Thread %i)Connected Socket Closed.",((threadInfo*)info)->thread_bin_index);
    numClients--;
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
  int i;
  uint8_t numClients = 0;
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

  // Enable Signal Handler
  signal(SIGTERM, catch_term);

  /* Open Log File Here */
  openlog("SEASSERVER",LOG_PID,LOG_DAEMON);
  syslog(LOG_DAEMON|LOG_INFO,"Daemon Started.");

  /* Open Config File Here */
  if(!readConfig()){
    syslog(LOG_DAEMON|LOG_ERR,"Unable to Read Configuration File.  Daemon Terminated.\n");
    exit(EXIT_FAILURE);
  }

  /* Create a new SID for the child process */
  sid = setsid();
  if(sid < 0){
    syslog(LOG_DAEMON|LOG_ERR,"Unable to create a new SID for child process. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if((chdir("/")) < 0){
    syslog(LOG_DAEMON|LOG_ERR,"Unable to switch working directory. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* Setup TCP/IP Socket */
  if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      syslog(LOG_DAEMON|LOG_ERR,"Unable to create socket. Daemon Terminated.");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

#ifdef DEBUG
  syslog(LOG_DAEMON|LOG_INFO,"Opening Spectrometers.");
#endif
  // Power Cycle GPIO for USB Hub to make sure USB ports initialized correctly
  //system("echo 0 > /sys/class/gpio/gpio168/value");
  //sleep(3);
  //system("echo 1 > /sys/class/gpio/gpio168/value");
  //sleep(3);
  // Connect the USB Spectrometers
  char *serialNumbers[NUM_SPECS] = {getSerialNumber(0),getSerialNumber(1)};
  if(connectSpectrometers(serialNumbers) == CONNECT_ERR)
  {
      syslog(LOG_DAEMON|LOG_ERR,"Spectrometers could not be opened.  Daemon Exiting");
      exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  syslog(LOG_DAEMON|LOG_INFO,"Spectrometers Opened.");
#endif

  applyConfig();
  logConfig();

  // Start LON Dispatch
  syslog(LOG_DAEMON|LOG_INFO,"Starting LON Connection.");
  if(startDispatch(LONPORT) == -1){
      syslog(LOG_DAEMON|LOG_ERR,"LON Not Connected!  Check serial port.");
      exit(EXIT_FAILURE);
  } else {
    syslog(LOG_DAEMON|LOG_INFO,"LON Connection Started.");
  }

  // Init Bench Config and Power Management GPIOs (See SEASPeriperalCommands.c)
  readBenchConfig();
  initPeripherals();

  // Start CTD Sink
  CTDSink* ctdSink = CTDSink::Instance();

  /*  Bind our socket addresss to the
	listening socket, and call listen()  */

  if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    syslog(LOG_DAEMON|LOG_ERR,"Unable to bind socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  if ( listen(list_s, NUM_THREADS) < 0 ) {
    syslog(LOG_DAEMON|LOG_ERR,"Unable to listen on socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  for(i=0; i < NUM_THREADS; i++)
      thread_bin_available[i] = AVAILABLE;

  execMode();

  while(keep_going){
    syslog(LOG_DAEMON|LOG_INFO,"Listening for connection on port %i", port);
    /* Wait for TCP/IP Connection */
    conn_s = accept(list_s, NULL, NULL);
    if ( conn_s < 0 ) {
        syslog(LOG_DAEMON|LOG_ERR,"Unable to call accept() on socket.");
        break;
    }

    /* Spawn a POSIX Server Thread to Handle Connected Socket */
    for(i=0; i < NUM_THREADS; i++){
        if(thread_bin_available[i]){
            thread_bin_available[i] = UNAVAILABLE;
            syslog(LOG_DAEMON|LOG_INFO,"Handling new connection on port %i",port);
            numClients++;
            info = (threadInfo*)malloc(sizeof(threadInfo));
            info->socket_connection = conn_s;
            info->thread_bin_index = i;
            pthread_create(&thread_bin[i],NULL,handleConnection, (void*)info);
            break;
        }
    }

    if(i > NUM_THREADS){
        syslog(LOG_DAEMON|LOG_ERR,"Unable to create thread to handle connection.  Continuing...");
    }
    
  }
  
  if(disconnectSpectrometers() == CONNECT_OK)
    syslog(LOG_DAEMON|LOG_INFO,"Spectrometers Successfully Disconnected");
  else
    syslog(LOG_DAEMON|LOG_ERR,"Unable to Disconnect Spectrometers");

  syslog(LOG_DAEMON|LOG_INFO,"Daemon Exited Politely.");
  exit(EXIT_SUCCESS);

}


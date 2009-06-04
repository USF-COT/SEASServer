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

#define CONFIGPATH "/media/card/StixServerConfig.txt"

int main(){
  pid_t pid, sid;
  int       list_s;                /*  listening socket          */
  int       conn_s;                /*  connection socket         */
  short int port = 1994;                  /*  port number               */
  struct    sockaddr_in servaddr;  /*  socket address structure  */


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

  /*  Bind our socket addresss to the
	listening socket, and call listen()  */

  if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    syslog(LOG_DAEMON||LOG_ERR,"Unable to bind socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  if ( listen(list_s, 128) < 0 ) {
    syslog(LOG_DAEMON||LOG_ERR,"Unable to listen on socket. Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  while(1){
    syslog(LOG_DAEMON||LOG_INFO,"Listening for connection on port %i", port);
    /* Wait for TCP/IP Connection */
    if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
        syslog(LOG_DAEMON||LOG_ERR,"Unable to call accept() on socket. Daemon Terminated.");
        exit(EXIT_FAILURE);
    }

    /* Spawn a POSIX Server Thread to Handle Connected Socket */
    syslog(LOG_DAEMON||LOG_INFO,"Handling new connection on port %i",port);
    if ( close(conn_s) < 0 ) {
        syslog(LOG_DAEMON||LOG_ERR,"Error calling close() on connection socket. Daemon Terminated.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DAEMON||LOG_INFO,"Connected Socket Closed.");
  }
  

  syslog(LOG_DAEMON||LOG_INFO,"Daemon Exited.");
  exit(EXIT_SUCCESS);

}


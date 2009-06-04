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
 *
 * Created on May 29, 2009, 2:40 PM
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#define CONFIGPATH "/media/card/StixServerConfig.txt"

int main(){
  pid_t pid, sid;
  FILE* configFile;

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

  /* Open Log File Here */
  openlog("DTEST",LOG_PID,LOG_DAEMON);
  syslog(LOG_DAEMON||LOG_INFO,"Daemon Started.");

  /* Open Config File Here */
  if(!readConfig(CONFIGPATH)){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to Read Configuration File.  Daemon Terminated.");
    exit(EXIT_FAILURE);
  }

  /* Create a new SID for the child process */
  sid = setsid();
  if(sid < 0){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to create a new SID for child process.");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if((chdir("/")) < 0){
    syslog(LOG_DAEMON||LOG_ERR,"Unable to switch working directory.");
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  //while(1){
    /* Do some task here */
  //  sleep(30); /* wait 30 seconds */
  //}
  

  syslog(LOG_DAEMON||LOG_INFO,"Daemon Exited.");
  exit(EXIT_SUCCESS);

}


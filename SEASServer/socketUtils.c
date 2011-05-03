#include "socketUtils.h"

void setSocketTimeout(int connection, time_t tv_sec, suseconds_t tv_usec){
    struct timeval tv;
    tv.tv_sec = tv_sec;
    tv.tv_usec = tv_usec;

    if(setsockopt(connection,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))){
        syslog(LOG_DAEMON|LOG_ERR,"Error Setting Timeout for Socket");
    }
}


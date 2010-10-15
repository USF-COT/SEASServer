/* 
 * LONDispatch.h
 *
 */

#ifndef _LONDISPATCH_H
#define	_LONDISPATCH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include "LONprotocol.h"

#define DEFAULTLONPORT "/dev/ttyS0"

typedef struct LONRESPONSE{
    unsigned char deviceID;
    unsigned int numBytes;
    unsigned char commandID;
    unsigned char* data;
    unsigned char checkSum;
}LONresponse_s;

int startDispatch(char* port);
void stopDispatch();
LONresponse_s* sendLONCommand(unsigned char device, unsigned char command, unsigned int dataLength, unsigned char* data);
void freeLONResponse(LONresponse_s* response);


#ifdef	__cplusplus
}
#endif

#endif	/* _LONDISPATCH_H */


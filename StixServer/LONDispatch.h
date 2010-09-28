/* 
 * LONDispatch.h
 *
 */

#ifndef _LONDISPATCH_H
#define	_LONDISPATCH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "LONprotocol.h"

#define DEFAULTLONPORT "/dev/ttyS0"

int startDispatch(char* port);
void stopDispatch();
unsigned char* sendLONCommand(unsigned char device, unsigned char command, unsigned int dataLength, unsigned char* data);


#ifdef	__cplusplus
}
#endif

#endif	/* _LONDISPATCH_H */


/* 
 * LONDispatch.h
 *
 */

#ifndef _LONDISPATCH_H
#define	_LONDISPATCH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define DEFAULTLONPORT "/dev/ttyS0"

void startDispatch(char* port);
void stopDispatch();
void sendLONCommand(int port,char device, char command, unsigned int dataLength, unsigned char* data);


#ifdef	__cplusplus
}
#endif

#endif	/* _LONDISPATCH_H */


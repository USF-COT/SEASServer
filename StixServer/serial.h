/* 
 * File:   serial.h
 * Author: seraph
 *
 * Created on June 4, 2009, 1:17 PM
 */

#ifndef _SERIAL_H
#define	_SERIAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <termios.h>
#include <unistd.h>
#include <stdio.h>

char connect(int* fd);
char sendData(char* data, int length);
char disconnect(int* fd);

#ifdef	__cplusplus
}
#endif

#endif	/* _SERIAL_H */


/* 
 * File:   serial.c
 * Author: seraph
 *
 * Created on June 4, 2009, 1:15 PM
 */

#ifndef _SERIAL_C
#define	_SERIAL_C

#ifdef	__cplusplus
extern "C" {
#endif

char connect(int fd);
char sendData(char* data);
char disconnect(int fd);


#ifdef	__cplusplus
}
#endif

#endif	/* _SERIAL_C */


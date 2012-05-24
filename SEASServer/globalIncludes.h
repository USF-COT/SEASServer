/* GlobalIncludes.h - Because I'm sick of redefining bools and testing if they are already defined.
 *
 * By: Michael Lindemuth
 */

#ifndef GLOBALINCLUDES_H
#define GLOBALINCLUDES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef BOOL
    typedef enum bools{FALSE=0,TRUE=1}BOOL;
#endif
char* byteArrayToString(unsigned char* bytes, unsigned int length);
void copyReverseBytes(void* dest, const void* src, unsigned int numBytes);
void soundBeep();

#ifdef __cplusplus
 }
#endif

#endif

/* GlobalIncludes.h - Because I'm sick of redefining bools and testing if they are already defined.
 *
 * By: Michael Lindemuth
 */

#ifndef GLOBALINCLUDES_H
#define GLOBALINCLUDES_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "usleep-drv.h"
#include <fcntl.h>
#include <syslog.h>

#ifndef BOOL
typedef enum bool{FALSE=0,TRUE=1}BOOL;
#endif

int milliSleep(unsigned int millisecondsToSleep);

#endif

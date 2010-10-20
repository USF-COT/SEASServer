/****************************************************************************
*
*   Copyright (c) 2008 Dave Hylands     <dhylands@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
*   Alternatively, this software may be distributed under the terms of BSD
*   license.
*
*   See README and COPYING for more details.
*
****************************************************************************/
/**
*
*   @file   usleep-test.c
*
*   @brief  This file tests the usleep-drv kernel module.
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "usleep-drv.h"
#include <sys/timeb.h>

// ---- Functions -----------------------------------------------------------

//***************************************************************************
/**
*   Main entry point
*/

int main( int argc, char **argv )
{
    int                 usleepDev;
    const char         *usleepDevName = "/dev/usleep";
    struct timeb        startTime;
    struct timeb        endTime;
    int                 delayTime;
    int                 numIter;
    int                 count;
    time_t              deltaSecs;
    unsigned            deltaMsecs;

    if ( argc != 3 )
    {
        fprintf( stderr, "Usage: sleep-test delay numIter\n" );
        exit( 1 );
    }

    delayTime = atoi( argv[ 1 ]);
    numIter   = atoi( argv[ 2 ]);

    if (( usleepDev = open( usleepDevName, O_RDWR )) < 0 )
    {
        fprintf( stderr, "Error  opening '%s': %s\n", usleepDevName, strerror( errno ));
        exit( 1 );
    }

    // Wait for the time to roll over

    ftime( &endTime );
    do
    {
        ftime( &startTime );
    } while ( startTime.time == endTime.time );

    for ( count = 0; count < numIter; count++ )
    {
        if ( ioctl( usleepDev, USLEEP_IOCTL_MDELAY, delayTime ) != 0 )
        {
            fprintf( stderr, "ioctl for usleep failed\n" );
            break;
        }
    }
    ftime( &endTime );

    deltaSecs  = endTime.time - startTime.time;
    if ( endTime.millitm < startTime.millitm  )
    {
        deltaSecs--;
        deltaMsecs = 1000 + endTime.millitm - startTime.millitm;
    }
    else
    {
        deltaMsecs = endTime.millitm - startTime.millitm;
    }

    printf( "Took %u.%03u seconds to delay for %d usecs %d times\n",
            deltaSecs, deltaMsecs, delayTime, numIter );

    close( usleepDev );

    return 0;
}

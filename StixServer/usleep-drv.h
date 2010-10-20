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
****************************************************************************
*
*   This driver allows multiple GPIO pins to be monitored and allows a user
*   mode program to be notified when the pin changes.
*
****************************************************************************/

#if !defined( USLEEP_DRV_H )
#define USLEEP_DRV_H

/* ---- Include Files ----------------------------------------------------- */

#if defined( __KERNEL__ )
#   include <linux/types.h>
#   include <linux/ioctl.h>
#else
#   include <stdint.h>
#   include <sys/ioctl.h>
#endif


/* ---- Constants and Types ----------------------------------------------- */

// The ioctl "magic" is just some character value which is used to help
// detect when incorrect ioctl values are sent down to a driver.

#define USLEEP_IOCTL_MAGIC  'u'

/**
 *  Deefines for each of the ioctl commands. Note that since we want to reduce
 *  the possibility that a user mode program gets out of sync with a given 
 *  driver, we explicitly assign a value to each enumeration. This makes
 *  it more difficult to stick new ioctl's in the middle of the list.
 */

typedef enum
{
    USLEEP_CMD_FIRST                = 0x80,

    USLEEP_CMD_UDELAY               = 0x80,
    USLEEP_CMD_MDELAY               = 0x81,

    /* Insert new ioctls here                                               */

    USLEEP_CMD_LAST,

} USLEEP_CMD;

/*
 * Definitions for the actual ioctl commands
 */

#define USLEEP_IOCTL_UDELAY     _IO( USLEEP_IOCTL_MAGIC, USLEEP_CMD_UDELAY )    // arg is int
#define USLEEP_IOCTL_MDELAY     _IO( USLEEP_IOCTL_MAGIC, USLEEP_CMD_MDELAY )    // arg is int

#endif  // USLEEP_DRV_H


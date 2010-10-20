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
*   This driver allows the kernel mode udelay and mdelay calls to be made 
*   from user space. This is useful when the kernel only has jiffie resolution
*   on usleep.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/ioctls.h>

#include "usleep-drv.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define USLEEP_DEV_NAME "usleep"

#define DEBUG_ENABLED   0

#if DEBUG_ENABLED
#   define DEBUG( flag, fmt, args... ) do { if ( gDebug ## flag ) printk( "%s: " fmt, __FUNCTION__ , ## args ); } while (0)
#else
#   define DEBUG( flag, fmt, args... )
#endif  

/* ---- Private Variables ------------------------------------------------ */

static char gBanner[] __initdata = KERN_INFO "uSleep Driver 0.1 Compiled: " __DATE__ " at " __TIME__ "\n";

#if DEBUG_ENABLED
static  int             gDebugTrace = 1;
static  int             gDebugError = 1;
#endif

static  struct class   *gUSleepClass = NULL;
static  struct  cdev    gUSleepCDev;
static  dev_t           gUSleepDevNum = 0;

/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*   usleep_ioctl
*
*   Called to process ioctl requests
*
*****************************************************************************/

int usleep_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg )
{
    DEBUG( Trace, "type: '%c' cmd: 0x%x\n", _IOC_TYPE( cmd ), _IOC_NR( cmd ));

    switch ( cmd )
    {
        case USLEEP_IOCTL_UDELAY:
        {
            DEBUG( Trace, "delaying for %lu usecs\n", arg );
            if ( arg > 0 )
            {
                udelay( arg );
            }
            break;
        }

        case USLEEP_IOCTL_MDELAY:
        {
            DEBUG( Trace, "delaying for %lu msecs\n", arg );
            if ( arg > 0 )
            {
                if ( arg <= MAX_UDELAY_MS )
                {
                    mdelay( arg );
                }
                else
                {
                    msleep_interruptible( (unsigned)arg );
                }
            }
            break;
        }

        default:
        {
            DEBUG( Error, "Unrecognized ioctl: '0x%x'\n", cmd );
            return -ENOTTY;
        }
    }

    return 0;

} // usleep_ioctl

/****************************************************************************
*
*  usleep_open
*
****************************************************************************/

static int usleep_open( struct inode *inode, struct file *file )
{
    DEBUG( Trace, "usleep_open called, major = %d, minor = %d\n", MAJOR( inode->i_rdev ),  MINOR( inode->i_rdev ));

    return 0;

} // usleep_open

/****************************************************************************
*
*  usleep_release
*
****************************************************************************/

static int usleep_release( struct inode *inode, struct file *file )
{
    DEBUG( Trace, "usleep_release called\n" );

    return 0;

} // usleep_release

/****************************************************************************
*
*   File Operations (these are the device driver entry points)
*
****************************************************************************/

struct file_operations usleep_fops =
{
    owner:      THIS_MODULE,
    ioctl:      usleep_ioctl,
    open:       usleep_open,
    release:    usleep_release,
};

/****************************************************************************
*
*  usleep_init
*
*     Called to perform module initialization when the module is loaded
*
****************************************************************************/

static int __init usleep_init( void )
{
    int rc;

    DEBUG( Trace, "called\n" );

    printk( gBanner );

    // Get a major number

    if (( rc = alloc_chrdev_region( &gUSleepDevNum, 0, 1, USLEEP_DEV_NAME )) < 0 )
    {
        printk( KERN_WARNING "sample: Unable to allocate major, err: %d\n", rc );
        return rc;
    }
    DEBUG( Trace, "allocated major:%d minor:%d\n", MAJOR( gUSleepDevNum ), MINOR( gUSleepDevNum ));

    // Register our device. The device becomes "active" as soon as cdev_add 
    // is called.

    cdev_init( &gUSleepCDev, &usleep_fops );

    if (( rc = cdev_add( &gUSleepCDev, gUSleepDevNum, 1 )) != 0 )
    {
        printk( KERN_WARNING "sample: cdev_add failed: %d\n", rc );
        return rc;
    }

    // Create a class, so that udev will make the /dev entry

    gUSleepClass = class_create( THIS_MODULE, USLEEP_DEV_NAME );
    if ( IS_ERR( gUSleepClass ))
    {
        printk( KERN_WARNING "sample: Unable to create class\n" );
        return -1;
    }

    class_device_create( gUSleepClass, NULL, gUSleepDevNum, NULL, USLEEP_DEV_NAME );

    return 0;

} // usleep_init

/****************************************************************************
*
*  usleep_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
****************************************************************************/

static void __exit usleep_exit( void )
{
    DEBUG( Trace, "called\n" );

    // Deregister our driver

    class_device_destroy( gUSleepClass, gUSleepDevNum );
    class_destroy( gUSleepClass );

    cdev_del( &gUSleepCDev );

    unregister_chrdev_region( gUSleepDevNum, 1 );

} // usleep_exit

/****************************************************************************/

module_init(usleep_init);
module_exit(usleep_exit);

MODULE_AUTHOR("Dave Hylands");
MODULE_DESCRIPTION("uSleep Driver");
MODULE_LICENSE("Dual BSD/GPL");


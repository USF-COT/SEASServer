/* A2DMax11615 - Responsible for handling all I2C communication with Maxim MAX11615.  Can configure, request, and retrieve data from specified device.
 *
 * By: Michael Lindemuth
 */

#ifndef A2DMAX11615
#define A2DMAX11615

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "globalIncludes.h"

#include "i2c-dev.h" /* for I2C_SLAVE */


typedef enum SCANTYPE{ZEROUP,SAME,UPPERQUART,SINGLE}scanType;

typedef struct A2DMAX11615{
    BOOL connected;
    int port;
    BOOL internRef;
    BOOL refOut;
    BOOL intRefSt;
    BOOL extClock;
    BOOL bipolar;
    scanType scan;
    unsigned short maxChannel;
    BOOL singleEnded;
}a2dMAX11615;

a2dMAX11615* connectMAX11615(const char* port);
int setSetupParams(a2dMAX11615* a2d, BOOL internRef,BOOL refOut, BOOL intRefSt, BOOL extClock, BOOL bipolar);
int setConfigParams(a2dMAX11615* a2d, scanType scan, unsigned short maxChannel, BOOL singleEnded);

// Reads in all channels up to maxChannel then returns them as an array terminated by a NULL pointer element.
int* readChannels(a2dMAX11615* a2d);
void disconnectMAX11615(a2dMAX11615* a2d);

#endif

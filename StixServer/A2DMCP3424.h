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

typedef enum MCP3424CONVMODE {ONESHOT=0,CONTINUOUS=1}MCP3424ConvMode;
typedef enum MCP3424SAMPRATE {SPS240=0,SPS60=1,SPS15=2,SPS3_75=3}MCP3424SampRate;
typedef enum MCP3424PGAGAIN  {PGAX1=0,PGAX2=1,PGAX4=2,PGAX8=3}MCP3424PGAGain;

typedef struct A2DMCP3424{
    char* port;
    uint8_t addressBits;
    uint8_t channel;
    MCP3424ConvMode mode; 
    MCP3424SampRate rate;
    MCP3424PGAGain gain;
}a2dMCP3424;

a2dMCP3424* createMCP3424(const char* port,uint8_t addressBits);
void destroyMCP3424(a2dMCP3424* handle);

void setConfiguration(a2dMCP3424* handle, BOOL RDY, uint8_t channel, MCP3424ConvMode mode, MCP3424SampRate rate, MCP3424PGAGain gain);

// Reads in all channels up to maxChannel then returns them as an array terminated by a NULL pointer element.
int* readChannel(a2dMCP3424* a2d,uint8_t channel);

#endif

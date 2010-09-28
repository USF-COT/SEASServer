/* 
 * File:   SEASStatus.h
 * Author: seraph
 *
 * Created on June 10, 2009, 11:06 AM
 */

#include "../USB4000Gum/source/USB4000Gum.h"
#include "globalIncludes.h"

#ifndef _SEASSTATUS_H
#define	_SEASSTATUS_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum{MANUAL,PROGRAM}SEASMODE;

    typedef struct PUMP{
        TOGGLE power;
        unsigned short pumpRPM;
    }pump;

    typedef struct HEATER{
        TOGGLE power;
        float temperature;
    }heater;

    typedef struct VALVE{
        VALVESTATUS status;
    }valve;

    typedef struct SEASSTATUS{
        SEASMODE mode;

        unsigned char numPumps;
        pump* pumps;
        unsigned char numHeaters;
        heater* heaters;
        unsigned char numValves;
        valve* valves;
        unsigned char numSpectrometers;
        spectrometer* spectrometers;
    }SEASStatus;

    SEASStatus* initStatus(SEASMODE mode,unsigned char numPumps, unsigned char numHeaters, unsigned char numValves, unsigned char numSpectrometers);


#ifdef	__cplusplus
}
#endif

#endif	/* _SEASSTATUS_H */


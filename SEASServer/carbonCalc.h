/* CarbonCalc.h - Defines structures and functions to be used in calculating total DIC, pH, and PC02.
 *
 * By: Jim Patten and Michael Lindemuth
 */

#ifndef CARBONCALC_H
#define CARBONCALC_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <syslog.h>
#include "salinity.h"
#include "SEASPeripheralCommands.h"

/* Default system temperature */
#define  DEFAULT_SYSTEM_TEMPERATURE    25.0 + ABSOLUTE_ZERO_TEMPERATURE


/* pH computation constants */
#define  pHC1              4.706
#define  pHC2              26.33
#define  pHC3              7.17218
#define  pHC4              0.017316
#define  pHC5              0.0035
#define  pHC6              2.3875
#define  pHC7              0.1387

/* Ct computation constants */
#define  CtS1_DEFAULT      2.6369
#define  Ct1               2.303
#define  Ct2               0.023517
#define  Ct3               0.023655
#define  Ct4               100
#define  Ct5               0.0047036
#define  Ct6               0.00387
#define  Ct7               0.006333


/* pCO2 computation constants */
#define  pCO2S1_DEFAULT    175.7
#define  pCO2S2_DEFAULT    46.064
#define  pCO21             8.03412
#define  pCO22             0.00244
#define  pCO23             2.734
#define  pCO24             0.1075
#define  K0                0.033884
#define  K1                0.0000004592
#define  K2                0.0000000000468813

// Base Methods
float computeSystemTotalCarbon(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS* CTDReading);
float computeSystempCO2(unsigned char absWaveCount,float* absorbance,float nonAbsWave);
float computeSystempHMCP(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS* CTDReading);
float computeSystempHTB(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS* CTDReading);
float computeAbsorbanceRatio(unsigned char absorbingWaveCount, float* absorbance, float nonAbsWave);

#endif

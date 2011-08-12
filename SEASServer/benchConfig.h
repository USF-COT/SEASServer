/* BenchConfig.h - Defines structures and methods for storing and retrieving benchtop configuration
 * parameters.  Also provides interfaces for parseGUI methods.
 *
 * By: Michael Lindemuth and Jim Patten
 * University of South Florida
 * Center for Ocean Technology
 */

#ifndef BENCHCONFIG_H
#define BENCHCONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "USB4000Manager.h"
#include "SEASPeripheralCommands.h"

/* Benchtop pump configuration data */
typedef  struct   {

  uint16_t   RPM;

}__attribute__((packed)) PUMP_CONFIG_DATA;

/* Absorbance range data */
typedef  struct   {

  double         Minimum;
  double         Maximum;

}__attribute__((packed)) ABSORBANCE_RANGE_DATA;

/* Benchtop configuration data */
typedef  struct   {

  PUMP_CONFIG_DATA        PumpData[6];
  float                   HeaterSetPointTemperature;
  ABSORBANCE_RANGE_DATA   RangeData[2];

}__attribute__((packed)) BENCH_CONFIG_DATA;

/* Save benchtop configuration */
typedef  struct   {

  unsigned char Command;
  BENCH_CONFIG_DATA BenchtopConfiguration;

}__attribute__((packed)) SAVE_BENCHTOP_CONFIG_MSG;

void readBenchConfig();

void receiveSaveBenchConfig(int connection, char* command);
void sendBenchConfig(int connection, char* command);

uint16_t getBenchPumpRPM(uint8_t pumpID);
float getBenchHeaterSetPoint();
float getBenchAbsRangeMin(uint8_t specID);
float getBenchAbsRangeMax(uint8_t specID);

#endif

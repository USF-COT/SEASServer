/*
  Run Protocol.h

  Mini SEAS GUI <-> Mini SEAS run time protocol definition header file.

  Revision History:

  4/11/2011   Jim Patten        Created.
  3/21/2011   Michael Lindemuth  Added byte packing attribute to avoid confusion when transmitting structures.
*/

#ifndef RUN_PROTO_H
#define RUN_PROTO_H

/* Static Size of Scan Size */
#define USB4000_NUMPIXELS 3840
#define MAX_ABSORBANCE_WAVELENGTHS 9

/* Run time message header byte */
#define  RTH   0xF0

#include "USB4000Gum.h"

/* Run time message types */
enum  RUN_TIME_MESSAGE_TYPES     {
  PUMP_ON_RUNTIME_CMD = 0,
  PUMP_OFF_RUNTIME_CMD,
  LAMP_ON_RUNTIME_CMD,
  LAMP_OFF_RUNTIME_CMD,
  HEATER_ON_RUNTIME_CMD,
  HEATER_OFF_RUNTIME_CMD,
  READ_REFERENCE_RUNTIME_CMD,
  READ_SAMPLE_RUNTIME_CMD,
  CAL_CONCENTRATION_RUNTIME_CMD,
  CAL_PCO2_RUNTIME_CMD,
  CAL_PH_RUNTIME_CMD,
  CAL_TC_RUNTIME_CMD,
  READ_FULL_SPECTRUM_RUNTIME_CMD,
  DELAY_RUNTIME_CMD,
  WAIT_HEATER_RUNTIME_CMD,
  READ_TEMPERATURE_RUNTIME_CMD,
  SET_DWELL_RUNTIME_CMD,
  METHOD_COMPLETED_CMD,
  MAX_RUNTIME_COMMANDS
};

// Standard Header
typedef struct {
    unsigned char HeadByte;
    unsigned char Command;
}__attribute__((packed)) RUNTIME_RESPONSE_HEADER;

/* Set dwell */
typedef  struct   {
  RUNTIME_RESPONSE_HEADER Header;
  unsigned char Spectrometer;
  uint16_t   Seconds;
}__attribute__((packed)) SET_DWELL_RUNTIME_DATA;


/* Runtime messages */

/* Pump on */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   uint16_t Pump;
   uint16_t Percent;
}__attribute__((packed)) PUMP_ON_RUNTIME_DATA;

/* Pump off */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   uint16_t Pump;
}__attribute__((packed)) PUMP_OFF_RUNTIME_DATA;

/* Heater on */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   float  Temperature;
}__attribute__((packed)) HEATER_ON_RUNTIME_DATA;

/* Calculate concentration */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   unsigned char Spectrometer;
   float Absorbance;
   float Concentration[MAX_ABSORBANCE_WAVELENGTHS];
   float RRatio;
}__attribute__((packed)) CAL_CONCENTRATION_RUNTIME_DATA;

/* Read reference */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   unsigned char Spectrometer;
   float Counts[ USB4000_NUMPIXELS ];
}__attribute__((packed)) READ_REFERENCE_RUNTIME_DATA; 

/* Read sample */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   unsigned char Spectrometer;
   float Counts[ USB4000_NUMPIXELS ];
   float Absorbance[ MAX_ABSORBANCE_WAVELENGTHS ];
   float AbsorbanceSpectra [ USB4000_NUMPIXELS ];
   float CorrectionAbsorbance;
}__attribute__((packed)) READ_SAMPLE_RUNTIME_DATA;

/* Delay */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   uint16_t SecondsRemaining;
}__attribute__((packed)) DELAY_RUNTIME_DATA;


/* Wait heater */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   float  Temperature;
   uint16_t Seconds;
}__attribute__((packed)) WAIT_HEATER_RUNTIME_DATA;


/* Read temperature */
typedef  struct   {
   RUNTIME_RESPONSE_HEADER Header;
   float  Temperature;
}__attribute__((packed)) READ_TEMPERATURE_RUNTIME_DATA;

#endif


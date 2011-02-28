/*
  Run Protocol.h

  Mini SEAS GUI <-> Mini SEAS run time protocol definition header file.

  Revision History:

  4/11/2011   Jim Patten        Created.
*/

/* Run time message header byte */
#define  RTH   0xF0

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
  MAX_RUNTIME_COMMANDS
};

/* Set dwell */
typedef  struct   {

  uint16   Seconds;

}SET_DWELL_RUNTIME_DATA;


/* Runtime messages */

/* Pump on */
typedef  struct   {

   uint16 Pump;
   float  RPM;
   
}PUMP_ON_RUNTIME_DATA;

/* Pump off */
typedef  struct   {

   uint16 Pump;
   
}PUMP_OFF_RUNTIME_DATA;

/* Heater on */
typedef  struct   {

   float  Temperature;
   
}HEATER_ON_RUNTIME_DATA;

/* Calculate concentration */
typedef  struct   {

   uchar Spectrometer;
   float Concentration;
   float RRatio;
   
}CAL_CONCENTRATION_RUNTIME_DATA;

/* Read reference */
typedef  struct   {

   uchar Spectrometer;
   float Counts[ USB4000_PIXELS ];

}READ_REFERENCE_RUNTIME_DATA;

/* Read sample */
typedef  struct   {

   uchar Spectrometer;
   float Counts[ USB4000_PIXELS ];
   float Absorbance[ MAX_ABSORBANCE_WAVELENGTHS ];
   float CorrectionAbsorbance;

}READ_SAMPLE_RUNTIME_DATA;

/* Delay */
typedef  struct   {

   uchar  Spectrometer;
   uint16 Seconds;

}DELAY_RUNTIME_DATA;


/* Wait heater */
typedef  struct   {

   float  Temperature;
   uint16 Seconds;

}WAIT_HEATER_RUNTIME_DATA;


/* Read temperature */
typedef  struct   {

   float  Temperature;

}READ_TEMPERATURE_RUNTIME_DATA;


/* Runtime data record */
typedef  union   {

   PUMP_ON_RUNTIME_DATA                      PumpOnData;
   PUMP_OFF_RUNTIME_DATA                     PumpOffData;
   HEATER_ON_RUNTIME_DATA                    HeaterOnData;
   CAL_CONCENTRATION_RUNTIME_DATA            ConcentrationData;
   READ_REFERENCE_RUNTIME_DATA               ReferenceData;
   READ_SAMPLE_RUNTIME_DATA                  SampleData;
   DELAY_RUNTIME_DATA                        DelayData;
   WAIT_HEATER_RUNTIME_DATA                  WaitHeaterData;
   READ_TEMPERATURE_RUNTIME_DATA             ReadTemperatureData;
   
}RUNTIME_DATA;

/* Runtime command record */
typedef  struct   {

   int            Command;
   RUNTIME_DATA   Data;
   
}RUNTIME_COMMAND_RECORD;


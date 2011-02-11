/*
  Run Protocol.h

  Mini SEAS GUI <-> Mini SEAS run time protocol definition header file.

  Revision History:

  4/11/2011   Jim Patten        Created.
*/

/* Run time message header byte */
#define  RTH   0xF0

enum  RUN_TIME_MESSAGE_TYPES     {

   SET_SPECTROMETER_PARAMETERS_RUNTIME_CMD   =  0,
   SET_SAMPLE_WAVELENGTHS_RUNTIME_CMD,
   SET_CORRECTION_WAVELENGTH_RUNTIME_CMD,
   PUMP_ON_RUNTIME_CMD,
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
   READ_FULL_SPECTRUM_RUNTIME_CMD,
   DELAY_RUNTIME_CMD,
   MAX_RUN_TIME_MESSAGES
};


/* Runtime messages */

/* Set spectrometer parameters */                              
typedef  struct   {

   uchar       Command;
   uchar       Spectrometer;
   uint16      IntegrationTime;
   uint16      ScansPerSample;
   short int   Boxcar;

}SPECTROMETER_PARAMETERS_RUNTIME_MSG;

/* Set sample wavelegth */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   float  Wavelength;
   
}SET_SAMPLE_WAVELENGTH_RUNTIME_MSG;

/* Set correction wavelegth */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   float  Wavelength;
   
}SET_CORRECTION_WAVELENGTH_RUNTIME_MSG;

/* Pump on */
typedef  struct   {

   uchar  Command;
   uint16 Pump;
   float  RPM;
   
}PUMP_ON_RUNTIME_MSG;

/* Pump off */
typedef  struct   {

   uchar  Command;
   uint16 Pump;
   
}PUMP_OFF_RUNTIME_MSG;

/* Heater on */
typedef  struct   {

   uchar  Command;
   float  Temperature;
   
}HEATER_ON_RUNTIME_MSG;

/* Calculate concentration */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   double Concentration;
   
}CAL_CONCENTRATION_RUNTIME_MSG,

/* Calculate pCO2 */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   double pCO2;
   
}CAL_PCO2_RUNTIME_MSG,

/* Calculate concentration */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   double pH;
   
}CAL_PH_RUNTIME_MSG,

/* Read full spectrum */
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   float  Counts[ USB4000_PIXELS ];

}READ_FULL_SPECTRUM_RUNTIME_MSG;


/* Delay*/
typedef  struct   {

   uchar  Command;
   uchar  Spectrometer;
   uint16 Seconds;

}DELAY_RUNTIME_MSG;


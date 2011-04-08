/* CarbonCalc.c - Calculates totalDIC, pH, and PCO2 for the system in which the spectrometers are taking readings.
 *
 * By: Jim Patten and Michael Lindemuth
 */

#include "carbonCalc.h"

// Change these constants here if necessary
const float CtS1 = CtS1_DEFAULT;
const float pCO2S1 = pCO2S1_DEFAULT;
const float pCO2S2 = pCO2S2_DEFAULT;

/*
  void  ComputeSystemTotalCarbon( int Spectrometer )

  Computes the system total carbon.
*/
float computeSystemTotalCarbon(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS* CTDReading)
{
  float   AbsorbanceRatio;
  float   Temperature;
  float   Salinity;
  float   Term1;
  float   Term2;
  float   Term3;
  float   Term4;
  float   Term5;
  float   Term6;
  float   Term7;
  float   Term8;
  float   TotalDissolvedCarbon;

  /* Compute the absorbance ratio */
  AbsorbanceRatio = computeAbsorbanceRatio(absWaveCount,absorbance,nonAbsWave);

  /* Get the temperature */
  Temperature = CTDReading->temperature;

  /* Get the salinity */
  Salinity = (float) computeSalinity(CTDReading->conductivity, CTDReading->temperature, CTDReading->pressure);

  /* Compute the equation terms */
  Term1 = (float) ( Salinity / (float) Ct1 );
  Term2 = (float) (  (float) Ct2 - (float) ( (float) Ct3 * ( Temperature / (float) Ct4 ) ) );
  Term3 = (float) ( pow( (float) ( Temperature / (float) Ct4 ), (float) 2 ) );
  Term4 = (float) ( (float) Ct5 * Term3 );
  Term5 = (float) ( (float) ( Term1 * (float) ( Term2  + Term4 ) ) );
  Term6 = (float) ( AbsorbanceRatio - (float) Ct6 );
  Term7 = (float) ( (float) 1 - (float) ( AbsorbanceRatio * (float) Ct7 ) );
  Term8 = (float) ( Term5 - CtS1 - log10( (float) ( Term6 / Term7 ) ) );

  /* Compute the total carbon */
  TotalDissolvedCarbon = pow( (float) 10, Term8 );
  /* Convert to micromoles */
  TotalDissolvedCarbon *= (float) 1000000;

  return TotalDissolvedCarbon;
}

/*
  void  ComputeSystempCO2( int Spectrometer )

  Computes the system pCO2.
*/
float  computeSystempCO2(unsigned char absWaveCount,float* absorbance,float nonAbsWave)
{
  float   AbsorbanceRatio;
  float   pCO2pH;
  float   Term1;
  float   Term2;
  float   Term3;
  float   Term4;
  float   Term5;

  /* Compute the absorbance ratio */
  AbsorbanceRatio = computeAbsorbanceRatio(absWaveCount,absorbance,nonAbsWave);

  /* Compute the equation terms */
  Term1 = (float) ( (float) 2 * (float) K0 * (float) K1 * (float) K2 );
  Term2 = (float) ( AbsorbanceRatio - (float) pCO22 );
  Term3 = (float) ( (float) pCO23 - (float) ( AbsorbanceRatio * (float) pCO24 ) );
  pCO2pH = (float) ( (float) pCO21 + log10( Term2 / Term3 ) );
  Term4 = pow( (float) 10, (float) ( (float) 2 * pCO2pH ) );
  Term5 = (float) ( (float) K0 *(float) K1 * pow( (float) 10, pCO2pH ) );

  /* Compute the pCO2 */
  return (float) ( (float) ( pCO2S1 / (float) ( (float) ( Term1 * Term4 ) + Term5 ) ) + pCO2S2 );
  
}

/*
  void  ComputeSystempH( int Spectrometer )

  Computes the system pH.
*/
float computeSystempH(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS* CTDReading)
{
  float   AbsorbanceRatio;
  float   Temperature;
  float   Salinity;
  float   Term1;
  float   Term2;
  float   Term3;
  float   Term4;
  float   Term5;
  float   Term6;

  /* Compute the absorbance ratio */
  AbsorbanceRatio = computeAbsorbanceRatio(absWaveCount,absorbance,nonAbsWave);

  /* Get the temperature */
  Temperature = CTDReading->temperature;

  /* Get the salinity */
  Salinity = computeSalinity(CTDReading->conductivity,CTDReading->temperature,CTDReading->pressure);

  /* Compute the equation terms */
  Term1 = (float) ( (float) ( (float) pHC1 * Salinity ) / Temperature );
  Term2 = (float) ( (float) pHC2  - ( (float) pHC3 * log10( Temperature ) ) );
  Term3 = (float) ( (float) pHC4 * Salinity );
  Term4 = (float) ( AbsorbanceRatio- (float) pHC5 );
  Term5 = (float) ( (float) pHC6 - (float) ( (float) pHC7 * AbsorbanceRatio ) );
  Term6 = log10( Term4 / Term5 );

  /* Compute the pH */
  return (float) ( Term1 + Term2 - Term3 + Term6 );
}

/*
  float   ComputeAbsorbanceRatio( int Spectrometer )

  Computes the absorbance ratio for the spectrometer.
*/
float   computeAbsorbanceRatio(unsigned char absorbingWaveCount, float* absorbance, float nonAbsWave)
{
  float   absorbanceRatio = -1;

  if(absorbingWaveCount > 2){
      /* Compute the absorbance ratio */
      absorbanceRatio = (float) ( (absorbance[1] - nonAbsWave) / (absorbance[0] - nonAbsWave ) );
  } else {
      syslog(LOG_DAEMON|LOG_ERR,"Too few wavelengths (%d) passed to computeAbsorbanceRatio function.",absorbingWaveCount);
  }
  /* Return the absorbance ratio */
  return( absorbanceRatio );
}

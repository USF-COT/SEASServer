/* CarbonCalc.c - Calculates totalDIC, pH, and PCO2 for the system in which the spectrometers are taking readings.
 *
 * By: Jim Patten and Michael Lindemuth
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "carbonCalc.h"
#include "CTD.h"

/*
  void  ComputeSystemTotalCarbon( int Spectrometer )

  Computes the system total carbon.
*/
void  ComputeSystemTotalCarbon( int Spectrometer )
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

  /* Compute the absorbance ratio */
  AbsorbanceRatio = ComputeAbsorbanceRatio( Spectrometer );

  /* Get the temperature */
  Temperature = ComputationData.Temperature;

  /* Get the salinity */
  Salinity = CTDData.Salinity;

  /* Compute the equation terms */
  Term1 = (float) ( Salinity / (float) Ct1 );
  Term2 = (float) (  (float) Ct2 - (float) ( (float) Ct3 * ( Temperature / (float) Ct4 ) ) );
  Term3 = (float) ( pow( (float) ( Temperature / (float) Ct4 ), (float) 2 ) );
  Term4 = (float) ( (float) Ct5 * Term3 );
  Term5 = (float) ( (float) ( Term1 * (float) ( Term2  + Term4 ) ) );
  Term6 = (float) ( AbsorbanceRatio - (float) Ct6 );
  Term7 = (float) ( (float) 1 - (float) ( AbsorbanceRatio * (float) Ct7 ) );
  Term8 = (float) ( Term5 - ComputationData.CtS1 - log10( (float) ( Term6 / Term7 ) ) );

  /* Compute the total carbon */
  TotalDissolvedCarbon = pow( (float) 10, Term8 );
  /* Convert to micromoles */
  TotalDissolvedCarbon *= (float) 1000000;
}

/*
  void  ComputeSystempCO2( int Spectrometer )

  Computes the system pCO2.
*/
void  ComputeSystempCO2( int Spectrometer )
{
  float   AbsorbanceRatio;
  float   pCO2pH;
  float   Term1;
  float   Term2;
  float   Term3;
  float   Term4;
  float   Term5;

  /* Compute the absorbance ratio */
  AbsorbanceRatio = ComputeAbsorbanceRatio( Spectrometer );

  /* Compute the equation terms */
  Term1 = (float) ( (float) 2 * (float) K0 * (float) K1 * (float) K2 );
  Term2 = (float) ( AbsorbanceRatio - (float) pCO22 );
  Term3 = (float) ( (float) pCO23 - (float) ( AbsorbanceRatio * (float) pCO24 ) );
  pCO2pH = (float) ( (float) pCO21 + log10( Term2 / Term3 ) );
  Term4 = pow( (float) 10, (float) ( (float) 2 * pCO2pH ) );
  Term5 = (float) ( (float) K0 *(float) K1 * pow( (float) 10, pCO2pH ) );

  /* Compute the pCO2 */
  pCO2 = (float) ( (float) ( ComputationData.pCO2S1 / (float) ( (float) ( Term1 * Term4 ) + Term5 ) ) + ComputationData.pCO2S2 );
}

/*
  void  ComputeSystempH( int Spectrometer )

  Computes the system pH.
*/
void  ComputeSystempH( int Spectrometer )
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
  AbsorbanceRatio = ComputeAbsorbanceRatio( Spectrometer );

  /* Get the temperature */
  Temperature = ComputationData.Temperature;

  /* Get the salinity */
  Salinity = CTDData.Salinity;

  /* Compute the equation terms */
  Term1 = (float) ( (float) ( (float) pHC1 * Salinity ) / Temperature );
  Term2 = (float) ( (float) pHC2  - ( (float) pHC3 * log10( Temperature ) ) );
  Term3 = (float) ( (float) pHC4 * Salinity );
  Term4 = (float) ( AbsorbanceRatio- (float) pHC5 );
  Term5 = (float) ( (float) pHC6 - (float) ( (float) pHC7 * AbsorbanceRatio ) );
  Term6 = log10( Term4 / Term5 );

  /* Compute the pH */
  pH = (float) ( Term1 + Term2 - Term3 + Term6 );
}

/*
  float   ComputeAbsorbanceRatio( int Spectrometer )

  Computes the absorbance ratio for the spectrometer.
*/
float   ComputeAbsorbanceRatio( int Spectrometer )
{
  float   Lambda1Absorbance;
  float   Lambda2Absorbance;
  float   CorrectionAbsorbance;
  float   AbsorbanceRatio;

  /* Compute the lambda 1 absorbance */
  Lambda1Absorbance = ComputeAbsorbance( Spectrometer, AbsorbanceWavelength[0] );

  /* Compute the lambda 2 absorbance */
  Lambda2Absorbance = ComputeAbsorbance( Spectrometer, AbsorbanceWavelength[1] );

  /* Compute the correction absorbance */
  CorrectionAbsorbance = ComputeAbsorbance( Spectrometer, CorrectionWavelength );

  /* Compute the absorbance ratio */
  AbsorbanceRatio =
     (float) ( ( Lambda2Absorbance - CorrectionAbsorbance ) / ( Lambda1Absorbance - CorrectionAbsorbance ) );

  /* Return the absorbance ratio */
  return( AbsorbanceRatio );
}

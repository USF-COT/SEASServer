/******************************* Absorbance calculation ******************************/

#include "absorbanceCalculation.h"

/*
   float ComputeAbsorbance(spectrometer* USB4000, unsigned short absorbingPixel, unsigned short nonAbsorbingPixel )
   
   Computes the absorbance for the specified spectrometer.
*/
float ComputeAbsorbance(spectrometer* USB4000, unsigned short absorbingPixel, unsigned short nonAbsorbingPixel)
{
   float Absorbance;
   float Fraction;
   float BaselineAbsorbance;
   float Numerator;
   float Denominator;

   if(USB4000->refSample == NULL || USB4000->darkSample == NULL || USB4000->sample == NULL)
      return -1;
   
   /* Compute the numerator */
   Numerator = (float) (USB4000->refSample->pixels[absorbingPixel] - USB4000->darkSample->pixels[absorbingPixel]);
   
   /* Compute the denominator */
   Denominator = (float) (USB4000->sample->pixels[absorbingPixel] - USB4000->darkSample->pixels[absorbingPixel] );
   
   /* Compute the absorbance */
   Fraction = Numerator/Denominator; 
   if(Fraction > 0)
       Absorbance = log10f(Fraction);
   else
       return Fraction;

   /* Compute baseline absorbance */
   BaselineAbsorbance = ComputeCorrectionAbsorbance(USB4000, nonAbsorbingPixel);
   
   /* Subtract baseline */
   Absorbance -= BaselineAbsorbance;

   /* Return the absorbance */
   return( Absorbance );
}

/*
   float ComputeCorrectionAbsorbance( int Spectrometer, unsigned int nonAbsorbingPixel )
   
   Computes the correction absorbance for the specified spectrometer.
*/
float ComputeCorrectionAbsorbance(spectrometer* USB4000, unsigned int nonAbsorbingPixel)
{
   float Numerator;
   float Denominator;
   float Fraction;
   float CorrectionAbsorbance;
   
   if(USB4000->refSample == NULL || USB4000->darkSample == NULL || USB4000->sample == NULL)
      return -1;

   /* Compute the numerator */
   Numerator = (float) ( USB4000->refSample->pixels[nonAbsorbingPixel] - USB4000->darkSample->pixels[nonAbsorbingPixel] );
   
   /* Compute the denominator */
   Denominator = (float) ( USB4000->sample->pixels[nonAbsorbingPixel] - USB4000->darkSample->pixels[nonAbsorbingPixel] );
   
   /* Compute the absorbance */
   Fraction = Numerator / Denominator;
   if(Fraction > 0)
       CorrectionAbsorbance = log10f( Numerator / Denominator );
   else
       return Fraction;

   /* Return the absorbance */
   return( CorrectionAbsorbance );
}


/* Number of pixels used to compute the dark value */
#define  DARK_PIXELS    22

/*
   float ComputeDarkValue( int Spectrometer )
   
   Computes the dark value for the spectrometer.
*/
float ComputeDarkValue(spectrometer* USB4000)
{
   float AverageDarkValue;
   float Accumulator;
   int   i;

   /* Initialize the accumulator */
   Accumulator = (float) 0;
   
   /* Sum the dark pixels */
   for( i = (int) 0; i < (int) DARK_PIXELS; i++ )
      Accumulator += USB4000->sample->pixels[ i ];
   
   /* Compute the average dark value */
   AverageDarkValue = (float) ( Accumulator / (float) DARK_PIXELS );

   /* Return the corrected average */
   return( NonLinearCountCorrection(USB4000, AverageDarkValue ) );
}


/* Non-linear coefficient count */
#define  NONLINEAR_COEFFICIENT_COUNT         8

/*
   float NonLinearCountCorrection( int Spectrometer, float Counts )
   
   Corrects the input spectrometer counts with the non-linear correction terms
   for the specified spectrometer.
*/
float NonLinearCountCorrection(spectrometer* USB4000, float Counts)
{
   unsigned short      i;
   double   NonLinearCorrection;
   double   PolynomialCountTerm;
   
   
   /* Initialize the non-linear correction */
   NonLinearCorrection = (double) 0;
   
   /* Compute the non-linear correction */
   for( i = (unsigned short) 0; i < (unsigned short) NONLINEAR_COEFFICIENT_COUNT; i++ )
      {
      /* Compute the polynomial count term */
      PolynomialCountTerm = (double) ( pow( Counts, (double) i ) );
      PolynomialCountTerm *= USB4000->calibration->nonLinearCorrectionOrder[i];
      /* Add to the non-linear correction */
      NonLinearCorrection += PolynomialCountTerm; 
      }

   /* Return the corrected counts */
   return( (double) ( Counts / NonLinearCorrection ) );
}


/************************** Spectrometer uitilities. *********************************************/

/*
   void  ComputeSpectrometerLambdaValues( unsigned char Spectrometer )
   
   Computes the lambda values for the specified spectrometer.
*/
void  ComputeSpectrometerLambdaValues(spectrometer* USB4000)
{
   int      i;
   double   FirstCoefficient;
   double   SecondCoefficient;
   double   ThirdCoefficient;
   double   Intercept;
   double   Term1;
   double   Term2;
   double   Term3;

   // Prepare for calculation by allocating a new lambdaValues array if necessary
   if(USB4000->lambdaValues == NULL)
     USB4000->lambdaValues = malloc(sizeof(double)*USB4000->status->numPixels);

   /* Get the coefficients */
   FirstCoefficient = (double) USB4000->calibration->wavelengthOrder[1];
   SecondCoefficient = (double) USB4000->calibration->wavelengthOrder[2];
   ThirdCoefficient = (double) USB4000->calibration->wavelengthOrder[3];

   /* Get the intercept */
   Intercept = (double)USB4000->calibration->wavelengthOrder[0];

   /* Compute the values for each pixel */
   for( i = (int) 0; i < (int) USB4000->status->numPixels; i++ )
      {
      /* Compute the terms */
      Term1 = (double) ( FirstCoefficient * (double) i );
      Term2 = (double) ( SecondCoefficient * pow( (double) i, (double) 2 ) );
      Term3 = (double) ( ThirdCoefficient * pow( (double) i, (double) 3 ) );
      
      /* Compute the lambda value */      
      USB4000->lambdaValues[i] = 
         (double) ( Term3 + Term2 + Term1 + Intercept ); 
         
      }
}

/*
   float GetCountsForWavelength( int Spectrometer, double Lambda )
   
   Returns the counts from the spectral data for the input wavelength.
*/
float GetCountsForWavelength(spectrometer* USB4000, double Lambda )
{
   unsigned short      i;
   double   PreviousLambda;
   double   NextLambda;

   // Make Sure Lambda is Initialized
   if(USB4000->lambdaValues == NULL)
      ComputeSpectrometerLambdaValues(USB4000);

   /* Search the lambda values array for the spectrometer */
   for( i = (unsigned short) 1; i < (unsigned short) USB4000->status->numPixels-1; i++ )
   {
      /* Get the previous lambda */
      PreviousLambda = USB4000->lambdaValues[ i-1 ];
      /* Get the next lambda */
      NextLambda = USB4000->lambdaValues[ i+1 ];
      /* Determine if this is the target lambda value */
      if( ( PreviousLambda < Lambda ) && ( Lambda <= NextLambda ) )
      {
         /* Target lambda found - return the spectral data for the lambda */
         return( NonLinearCountCorrection(USB4000, USB4000->sample->pixels[i]) );
      }
   }

   /* Target not found - return zero */
   return( (float) 0 );
}

/*
   int   GetPixelForWavelength( int Spectrometer, double Lambda )
   
   Returns the pixel number for the input wavelength.
*/
unsigned short   GetPixelForWavelength(spectrometer* USB4000, double Lambda )
{
   unsigned short      i;
   double   PreviousLambda;
   double   NextLambda;

   // Make Sure Lambda is Initialized
   if(USB4000->lambdaValues == NULL)
      ComputeSpectrometerLambdaValues(USB4000);

   /* Search the lambda values array for the spectrometer */
   for( i = (unsigned short) 1; i < (unsigned short) USB4000->status->numPixels-1; i++ )
   {
      /* Get the previous lambda */
      PreviousLambda = USB4000->lambdaValues[ i-1 ];
      /* Get the next lambda */
      NextLambda = USB4000->lambdaValues[ i+1 ];
      /* Determine if this is the target lambda value */
      if( ( PreviousLambda < Lambda ) && ( Lambda <= NextLambda ) )
      {
         /* Target lambda found - return the pixel number for the lambda */
         return( i );
      }
   }

   /* Target not found - return zero */
   return(0);
}



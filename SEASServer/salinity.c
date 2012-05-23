#include "salinity.h"

/* Salinity calculation exponent table */
SALINTIY_EXPONENTS   SalinityExponents =  {

    { (double) a0, (double) a1, (double) a2, (double) a3, (double) a4, (double) a5 },
    { (double) b0, (double) b1, (double) b2, (double) b3, (double) b4, (double) b5 },
    { (double) c0, (double) c1, (double) c2, (double) c3, (double) c4, (double) 0.00 },
    { (double) 0.00, (double) d1, (double) d2, (double) d3, (double) d4, (double) 0.00 },
    { (double) 0.00, (double) e1, (double) e2, (double) e3, (double) 0.00, (double) 0.00 }

};

/*
   double   ComputeSalinity( CTD_SENSOR_DATA   *CTDSensorData )

   Computes salinity from the input CTD data.
 */
double computeSalinity(double conductivity, double temperature, double pressure)
{
    double   Salinity = 0.00;
    double   R;
    double   Rp;
    double   Rt;
    double   rT;
    double   Temp1;
    double   Temp2;
    double   Temp3;
    double   DeltaS;
    int      i;

    /* Do not process zero conductivity */
    if( (double) conductivity < (double) 0.00 )
        return( (double) 0.00 );

    /* Compute R */
    R = (double) ( (double) conductivity / (double) STANDARD_CONDUCTIVITY );

    /* Compute Rp */
    Temp1 = (double) 0;
    for( i = (int) 1; i < (int) 4; i++ )
    {
        Temp1 += (double) ( SalinityExponents.evalues[i] *
                pow( (double) pressure, (double) ( i - (int) 1 ) ) );
    }
    Temp1 *= (double) pressure;
    Temp2 = (double) 0;
    for( i = (int) 1; i < (int) 3; i++ )
    {
        Temp2 += (double) ( SalinityExponents.dvalues[i] *
                pow( (double) temperature, (double) i ) );
    }
    Temp2 += (double) 1;
    Temp3 = (double) ( SalinityExponents.dvalues[3] + (double) ( SalinityExponents.dvalues[4] * temperature ) );
    Temp3 *= R;
    Rp = (double) ( (double) 1 + (double) ( Temp1 / (double) ( Temp2 + Temp3 ) ) );

    /* Compute rT */
    rT = (double) 0;
    for( i = (int) 0; i < (int) 5; i++ )
    {
        rT += (double) ( SalinityExponents.cvalues[i] * 
                pow( (double) temperature, (double) i ) );
    }

    /* Compute Rt */
    Rt = (double) ( R / (double) ( Rp * rT ) );

    /* Begin salinity computation */
    Temp1 = Temp2 = (double) 0;

    for( i = (int) 0; i < (int) MAX_SALINITY_EXPONENTS; i++ )
    {
        Temp1 += (double) ( SalinityExponents.avalues[i] *
                pow( Rt, (double) ( (double) i / (double) 2 ) ) );

        Temp2 += (double) ( SalinityExponents.bvalues[i] *
                pow( Rt, (double) ( (double) i / (double) 2 ) ) );
    }

    /* Compute DeltaS */
    Temp3 = (double) ( (double) ( temperature - (double) 15 ) );
    DeltaS = (double) ( Temp3 / (double) ( (double) 1 + ( double)  ( (double) k  * Temp3 ) ) );
    DeltaS *= Temp2;
    Salinity = (double) ( Temp1 + DeltaS );

    /* Return the computed salinity */
    return( Salinity );
}


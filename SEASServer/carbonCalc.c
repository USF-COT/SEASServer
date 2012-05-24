/* CarbonCalc.c - Calculates totalDIC, pH, and PCO2 for the system in which the spectrometers are taking readings.
 *
 * By: Jim Patten and Michael Lindemuth
 */

#include "carbonCalc.h"

#include <pthread.h>

// Change these constants here if necessary
const float CtS1 = CtS1_DEFAULT;
const float pCO2S1 = pCO2S1_DEFAULT;
const float pCO2S2 = pCO2S2_DEFAULT;

static pthread_mutex_t carbonMutex = PTHREAD_MUTEX_INITIALIZER;
static pHCalc lastCalc;

/*
   void  ComputeSystemTotalCarbon( int Spectrometer )

   Computes the system total carbon.
 */
float computeSystemTotalCarbon(unsigned char absWaveCount,float* absorbance,float nonAbsWave, struct CTDREADINGS ctd)
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
    Temperature = ctd.temperature;

    /* Get the salinity */
    Salinity = (float) computeSalinity(ctd.conductivity, ctd.temperature, ctd.pressure);

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

float computeSystempHMCP(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS ctd)
{
    float S;
    float T;
    float R;
    float a;
    float b;
    float c;
    float d;
    float E1;
    float E32;
    float Rt;
    float pH;

    if(absWaveCount >= 2){
        syslog(LOG_DAEMON|LOG_INFO,"Computing salinity. Cond: %g, Temp: %g, Press: %g",ctd.conductivity,ctd.temperature,ctd.pressure);
        S = (float) computeSalinity((double)ctd.conductivity,(double)ctd.temperature,(double)ctd.pressure);
        syslog(LOG_DAEMON|LOG_INFO,"Salinity computed.");
        T = 273.15+ctd.temperature;
        R = absorbance[1]/absorbance[0];
        a = -246.64209+0.315971*S+(2.8855*pow(10,-4))*pow(S,2);
        b = 7229.23864-7.098137*S-0.057034*pow(S,2);
        c = 44.493382-0.052711*S;
        d = 0.0781344;
        E1 = -0.007762+4.5174*(pow(10,-5))*T;
        E32 = -0.020813+2.60262*(pow(10,-4))*T + 1.0436*(pow(10,-4))*(S-35);
        Rt = (R-E1)/(1-R*E32);
        if(T == 0 || Rt == 0){
            syslog(LOG_DAEMON|LOG_ERR,"T (%g) or Rt (%g) is 0. Setting pH to -1",T,Rt);
            pH = -1;
        } else {
            pH = a + b/T + c*log(T) - d*T + log10(Rt);
        }
        syslog(LOG_DAEMON|LOG_INFO,"Computed pH=%g. S:%g, T: %g, R: %g, a: %g, b: %g, c: %g, d: %g, e1: %g, e3/e2: %g, R-term: %g, Abs(578): %g, Abs(434): %g",pH,S,T,R,a,b,c,d,E1,E32,Rt,absorbance[1],absorbance[0]);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Not enough waves passed for pH calculation.  Setting pH to -1");
        pH = -1;
    }
    pthread_mutex_lock(&carbonMutex);
    lastCalc.pH = pH;
    lastCalc.rRatio = R;
    pthread_mutex_unlock(&carbonMutex);
    return pH;
}
/*
   void  ComputeSystempH( int Spectrometer )

   Computes the system pH.
 */
float computeSystempHTB(unsigned char absWaveCount,float* absorbance,float nonAbsWave,struct CTDREADINGS ctd)
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
    float   pH;

    /* Compute the absorbance ratio */
    AbsorbanceRatio = computeAbsorbanceRatio(absWaveCount,absorbance,nonAbsWave);

    /* Get the temperature */
    Temperature = ctd.temperature;

    /* Get the salinity */
    Salinity = computeSalinity(ctd.conductivity,ctd.temperature,ctd.pressure);

    /* Compute the equation terms */
    Term1 = (float) ( (float) ( (float) pHC1 * Salinity ) / Temperature );
    Term2 = (float) ( (float) pHC2  - ( (float) pHC3 * log10( Temperature ) ) );
    Term3 = (float) ( (float) pHC4 * Salinity );
    Term4 = (float) ( AbsorbanceRatio- (float) pHC5 );
    Term5 = (float) ( (float) pHC6 - (float) ( (float) pHC7 * AbsorbanceRatio ) );
    Term6 = log10( Term4 / Term5 );

    /* Compute the pH */
    pH = (float) ( Term1 + Term2 - Term3 + Term6 );
    pthread_mutex_lock(&carbonMutex);
    lastCalc.pH = pH;
    lastCalc.rRatio = AbsorbanceRatio;
    pthread_mutex_unlock(&carbonMutex);
    return (float) ( Term1 + Term2 - Term3 + Term6 );
}

/*
   float   ComputeAbsorbanceRatio( int Spectrometer )

   Computes the absorbance ratio for the spectrometer.
 */
float   computeAbsorbanceRatio(unsigned char absorbingWaveCount, float* absorbance, float nonAbsWave)
{
    float   absorbanceRatio = -1;

    if(absorbingWaveCount >= 2){
        /* Compute the absorbance ratio */
        absorbanceRatio = (float) absorbance[1] / absorbance[0];
        // Correction already applied in getAbsorbance method
        //absorbanceRatio = (float) ( (absorbance[1] - nonAbsWave) / (absorbance[0] - nonAbsWave ) );
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Too few wavelengths (%d) passed to computeAbsorbanceRatio function.",absorbingWaveCount);
    }
    /* Return the absorbance ratio */
    return( absorbanceRatio );
}

pHCalc* getLastpHCalc(){
    pHCalc *retVal = (pHCalc*)malloc(sizeof(pHCalc));
    pthread_mutex_lock(&carbonMutex);
    *retVal = lastCalc;
    pthread_mutex_unlock(&carbonMutex);
    return retVal;
}


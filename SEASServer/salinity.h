/* Salinity.h - An extensible, short source code set meant to calculate salinity given conductivity temperature and depth.
 * 
 * Originally By: Jim Patten
 * Modified to Standalone File By: Michael Lindemuth
 * 
 * University of South Florida
 * College of Marine Science
 * Center for Ocean Technology
 */

#ifndef SALINITY_H
#define SALINITY_H

#include <math.h>

/* Salinity calculation constants */
#define  STANDARD_CONDUCTIVITY   42.914
#define  k                       0.0162
#define  a0                      0.0080
#define  a1                      -0.1692
#define  a2                      25.3851
#define  a3                      14.0941
#define  a4                      -7.0261
#define  a5                      2.7081
#define  b0                      0.0005
#define  b1                      -0.0056
#define  b2                      -0.0066
#define  b3                      -0.0375
#define  b4                      0.0636
#define  b5                      -0.0144
#define  c0                      0.6766097
#define  c1                      0.0200564
#define  c2                      0.0001104259
#define  c3                      -0.00000069698
#define  c4                      0.0000000010031
#define  d1                      0.03426
#define  d2                      0.0004464
#define  d3                      0.4215
#define  d4                      -0.003107
#define  e1                      0.0002070
#define  e2                      -0.00000006370
#define  e3                      0.000000000003989

/* Salintiy exponent table entry */
#define  MAX_SALINITY_EXPONENTS     6
typedef  struct   {

   double   avalues[ MAX_SALINITY_EXPONENTS ];
   double   bvalues[ MAX_SALINITY_EXPONENTS ];
   double   cvalues[ MAX_SALINITY_EXPONENTS ];
   double   dvalues[ MAX_SALINITY_EXPONENTS ];
   double   evalues[ MAX_SALINITY_EXPONENTS ];

}SALINTIY_EXPONENTS;

double computeSalinity(double conductivity, double temperature, double pressure);

#endif

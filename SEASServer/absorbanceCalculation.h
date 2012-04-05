/* Absorbance Calculations Header
 * - Used to calculate the absorbance from a USB4000 sample
 *
 * By: Jim Patten and Michael Lindemuth
 */

#ifndef _ABSORBANCE_H
#define _ABSORBANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libUSB4000.h"
#include "globalIncludes.h"

float ComputeAbsorbance(spectrometer* USB4000, unsigned short absorbingPixel, unsigned short nonAbsorbingPixel, BOOL nonLinearCorrect, BOOL subBaseline);

float ComputeCorrectionAbsorbance(spectrometer* USB4000, unsigned short nonAbsorbingPixel, BOOL nonLinearCorrect);

float ComputeDarkValue(spectrometer* USB4000, specSample* sample, BOOL nonLinearCorrect);

float NonLinearCountCorrection(spectrometer* USB4000, float Counts);
void  ComputeSpectrometerLambdaValues(spectrometer* USB4000);
float GetCountsForWavelength(spectrometer* USB4000, double Lambda );
unsigned short GetPixelForWavelength(spectrometer* USB4000, double Lambda );

#endif

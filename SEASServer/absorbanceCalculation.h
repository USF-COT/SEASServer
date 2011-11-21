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
#include <syslog.h>
#include "libUSB4000.h"

float ComputeAbsorbance(spectrometer* USB4000, unsigned short absorbingPixel, unsigned short nonAbsorbingPixel);
float ComputeCorrectionAbsorbance(spectrometer* USB4000, unsigned short nonAbsorbingPixel);
float ComputeDarkValue(spectrometer* USB4000, specSample* sample);
float NonLinearCountCorrection(spectrometer* USB4000, float Counts);
void  ComputeSpectrometerLambdaValues(spectrometer* USB4000);
float GetCountsForWavelength(spectrometer* USB4000, double Lambda );
unsigned short GetPixelForWavelength(spectrometer* USB4000, double Lambda );

#endif

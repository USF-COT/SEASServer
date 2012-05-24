/* Code to read values from the ADC onboard the Overo.
 *
 * Based on madc-test.c written by Hugo Vincent
 * <http://old.nabble.com/Re%3A-Help-with-Overo-ADC-td24174607.html>
 *
 * By: Michael Lindemuth
 */

#ifndef OVEROADC_H
#define OVEROADC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>


#include "GUIprotocol.h"

struct adc_channel {
        int number;
        char name[16];
        float input_range;
};

float getBatteryVoltage();
void receiveGetBatteryVoltage(int connection, char* command);

#ifdef __cplusplus
 }
#endif

#endif

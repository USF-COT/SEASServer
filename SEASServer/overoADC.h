/* Code to read values from the ADC onboard the Overo.
 *
 * Based on madc-test.c written by Hugo Vincent
 * <http://old.nabble.com/Re%3A-Help-with-Overo-ADC-td24174607.html>
 *
 * By: Michael Lindemuth
 */

#ifndef OVEROADC_H
#define OVEROADC_H

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include "twl4030-madc.h"

#include "GUIprotocol.h"

struct adc_channel {
        int number;
        char name[16];
        float input_range;
};

float getBatteryVoltage();
void receiveGetBatteryVoltage(int connection, char* command);

#endif

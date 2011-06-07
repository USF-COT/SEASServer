#include "overoADC.h"

/* Channel numbering:
 * ADC0-1 : to do with battery charging, not relevant on Overo
 *   ADC2-7 : general purpose, input range = 0 - 2.5V.
 *            On Overo, ADC2 seems to read as ~0.4 and ADC7 as ~1.4V (?).
 *   ADC8 : USB OTG port bus voltage.
 *   ADC9-11 : more battery charging stuff, not relevant.
 *   ADC12 : main battery voltage.
 *           This will be the system 3.3V rail in our case
 *   ADC13-15: reserved or not relevant.
 */

struct adc_channel channels[] = {
        {
                .number = 2,
                .name = "ADCIN2",
                .input_range = 2.5,
        },
        {
                .number = 3,
                .name = "ADCIN3",
                .input_range = 2.5,
        },
        {
                .number = 4,
                .name = "ADCIN4",
                .input_range = 2.5,
        },
        {
                .number = 5,
                .name = "ADCIN5",
                .input_range = 2.5,
        },
        {
                .number = 6,
                .name = "ADCIN6",
                .input_range = 2.5,
        },
        {
                .number = 7,
                .name = "ADCIN7",
                .input_range = 2.5,
        },
        {
                .number = 8,
                .name = "VBUS_USB_OTG",
                .input_range = 7.0,
        },
        {
                .number = 12,
                .name = "VBATT/3.3V_RAIL",
                .input_range = 6.0,
        },
        {
                .number = 0,
                .name = "",
                .input_range = 0,
        }
};

float getChannelVoltage(int channel){
    int i = 0;
    int dev,ret;

    float result = -1;
    struct adc_channel* selected = NULL;
    struct twl4030_madc_user_parms *par;

    while(channels[++i].number != 0){
        if(channels[i].number == channel){
            selected = &(channels[i]);
        }
    }

    if(selected){
        dev = open("/dev/twl4030-madc",O_RDWR | O_NONBLOCK);
        if(dev == -1){
            syslog(LOG_DAEMON|LOG_ERR,"Could not open /dev/twl4030-madc to read ADC.  Check OS kernel drivers and modules.");
            return -1;
        }
        par = malloc(sizeof(struct twl4030_madc_user_parms));

        memset(par,0,sizeof(struct twl4030_madc_user_parms));
        par->channel = selected->number;
        ret = ioctl(dev, TWL4030_MADC_IOCX_ADC_RAW_READ, par);        
        result = ((unsigned int)par->result) / 1024.f; // 10 bit ADC -> 1024

        free(par);

        if(ret == 0 && par->status != -1){
            result = result * selected->input_range;
            syslog(LOG_DAEMON|LOG_INFO,"%s (Channel %d): %f",selected->name,selected->number,result);
            return result;
        } else {
            syslog(LOG_DAEMON|LOG_ERR,"Error in ioctl call to TWL4030 device for channel %d",selected->number);
            return -1;
        }
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"Channel %d is not a valid ADC channel number.",channel);
        return -1;
    }
}

float getBatteryVoltage(){
    float rawVoltage = getChannelVoltage(3);

    if(rawVoltage != -1){
        return rawVoltage * 12;
    } else {
        return -1;
    }
}

void receiveGetBatteryVoltage(int connection, char* command){
    unsigned char sendBuffer[5];
    float batteryVoltage;

    if(command[0] == RBS){
        syslog(LOG_DAEMON|LOG_INFO,"Getting battery voltage for GUI");
        batteryVoltage = getBatteryVoltage();
        syslog(LOG_DAEMON|LOG_INFO,"Retrieved battery voltage as: %f.",batteryVoltage);
        sendBuffer[0] = RBS;
        memcpy(sendBuffer+1,&batteryVoltage,4);
        send(connection,sendBuffer,5,0);
    } else {
        syslog(LOG_DAEMON|LOG_ERR,"ERROR: Unrecognized command sent to receiveBatteryVoltage method.");
    }
}


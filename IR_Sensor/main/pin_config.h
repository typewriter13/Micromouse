#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "hal/adc_types.h"

#define NUM_IR_SENSORS 5

static const adc_channel_t IR_SENSOR_CHANNELS[NUM_IR_SENSORS] = {
    ADC_CHANNEL_0, // IR Sensor 1
    ADC_CHANNEL_1, // IR Sensor 2
    ADC_CHANNEL_2, // IR Sensor 3
    ADC_CHANNEL_3, // IR Sensor 4
    ADC_CHANNEL_4  // IR Sensor 5
};

#endif // PIN_CONFIG_H

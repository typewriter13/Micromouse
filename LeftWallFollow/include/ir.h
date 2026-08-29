
#ifndef PINDEF_H
#define PINDEF_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define IR_PIN_0 ADC_CHANNEL_2  //GPIO 3 Right
#define IR_PIN_1 ADC_CHANNEL_8  //GPIO 9 Left
#define IR_PIN_2 ADC_CHANNEL_9  //GPIO 10 Left Diagonal
#define IR_PIN_3 ADC_CHANNEL_7  //GPIO 8 Right Diagonal

extern adc_channel_t IR_PINS[4];
extern const char* sensor_names[4];

extern adc_oneshot_unit_handle_t adc1_handle;

void ir_configuration(void);

#endif

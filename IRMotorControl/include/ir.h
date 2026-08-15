#ifndef PINDEF_H
#define PINDEF_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define IR_PIN_0 ADC_CHANNEL_2  // Right
#define IR_PIN_1 ADC_CHANNEL_8  // Left
#define IR_PIN_2 ADC_CHANNEL_9  // Left Diagonal
#define IR_PIN_3 ADC_CHANNEL_7  // Right Diagonal

adc_channel_t IR_PINS[4] = {IR_PIN_0, IR_PIN_1, IR_PIN_2, IR_PIN_3};
const char* sensor_names[4] = {"Right", "Left", "Left Diag", "Right Diag"};

extern adc_oneshot_unit_handle_t adc1_handle;

#define IR_Deadband 200

#endif

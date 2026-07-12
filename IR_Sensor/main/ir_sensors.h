#ifndef IR_SENSORS_H
#define IR_SENSORS_H

#include "esp_err.h"
#include "pin_config.h"

esp_err_t init_ir_sensors(void);
esp_err_t read_ir_sensors(int readings[NUM_IR_SENSORS]);

#endif

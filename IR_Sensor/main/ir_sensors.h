#ifndef IR_SENSORS_H
#define IR_SENSORS_H

#include "esp_err.h"
#include "pin_config.h"

typedef struct {
    int readings[NUM_IR_SENSORS];
} ir_sensor_reading_t;

esp_err_t init_ir_sensors(void);

esp_err_t read_ir_sensors(ir_sensor_reading_t *reading);

#endif // IR_SENSORS_H

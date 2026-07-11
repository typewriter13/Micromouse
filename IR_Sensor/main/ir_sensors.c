#include "ir_sensors.h"
#include "esp_adc/adc_oneshot.h"

// TODO: a static variable to hold the ADC handle (so both functions below can use it)

esp_err_t init_ir_sensors(void) {
    // TODO: 1. create the ADC unit
    // TODO: 2. loop through NUM_IR_SENSORS, configure each channel using IR_SENSOR_CHANNELS[i]
    // TODO: 3. return ESP_OK (or an error if something failed)
}

esp_err_t read_ir_sensors(ir_sensor_reading_t *reading) {
    // TODO: loop through NUM_IR_SENSORS, call adc_oneshot_read() for each channel, store into reading->readings[i]
    // TODO: return ESP_OK
}

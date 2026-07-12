#include "ir_sensors.h"
#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t adc_handle;

esp_err_t init_ir_sensors(void) {
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (ret != ESP_OK) return ret;

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    for (int i = 0; i < NUM_IR_SENSORS; i++) {
        ret = adc_oneshot_config_channel(adc_handle, IR_SENSOR_CHANNELS[i], &chan_config);
        if (ret != ESP_OK) return ret;
    }
    return ESP_OK;
}

esp_err_t read_ir_sensors(int readings[NUM_IR_SENSORS]) {
    for (int i = 0; i < NUM_IR_SENSORS; i++) {
        esp_err_t ret = adc_oneshot_read(adc_handle, IR_SENSOR_CHANNELS[i], &readings[i]);
        if (ret != ESP_OK) return ret;
    }
    return ESP_OK;
}

#include "ir_sensors.h"
#include "ir_pins.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "IR_SENSOR";

static const adc_channel_t ir_channels[NUM_IR_SENSORS] = {
    IR1_CHANNEL, IR2_CHANNEL, IR3_CHANNEL, IR4_CHANNEL, IR5_CHANNEL
};

static adc_oneshot_unit_handle_t adc1_handle = NULL;

void ir_sensors_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t err = adc_oneshot_new_unit(&init_config, &adc1_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC unit: %s", esp_err_to_name(err));
        return;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i = 0; i < NUM_IR_SENSORS; i++) {
        err = adc_oneshot_config_channel(adc1_handle, ir_channels[i], &chan_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure channel %d: %s", ir_channels[i], esp_err_to_name(err));
        }
    }
}

void ir_sensors_read(int values[NUM_IR_SENSORS])
{
    for (int i = 0; i < NUM_IR_SENSORS; i++) {
        esp_err_t err = adc_oneshot_read(adc1_handle, ir_channels[i], &values[i]);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Read failed on channel %d: %s", ir_channels[i], esp_err_to_name(err));
            values[i] = -1;  // sentinel value to indicate a bad read
        }
    }
}
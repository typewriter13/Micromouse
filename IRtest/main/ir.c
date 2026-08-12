#include "pindef.h"
#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t adc1_handle;

void ir_configuration() {

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_PIN_0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_PIN_1, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_PIN_2, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_PIN_3, &config));
}

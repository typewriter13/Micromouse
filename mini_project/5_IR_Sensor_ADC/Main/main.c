#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "IR_1.h"
#include "IR2.h"
#include "IR_3.h"
#include "IR_4.h"
#include "IR_5.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t adc1_handle; 
void app_main(void)
{
	adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
        adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config);
adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);
 adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config);
adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config);

    while (true) {
       
     ir_1();
     ir_2();
     ir_3();
     ir_4();
     ir_5();
     
     vTaskDelay(600 / portTICK_PERIOD_MS);
     
    }
}

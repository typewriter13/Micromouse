#include "../include/pindef.h"
#include <esp_adc/adc_oneshot.h>

void app_main(void)
{
    int adc_value[4];
    ir_configuration();
    
    while(1) {
        for(int i = 0; i < 4; i++) {
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PINS[i], &adc_value[i]));
            printf("%s: %d ", sensor_names[i], adc_value[i]);
        }
        printf("\n");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

#include <stdbool.h>
#include <stdio.h>
#include "IR_3.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "hal/adc_types.h"


int ir3_adc=0;
extern adc_oneshot_unit_handle_t adc1_handle; 

void ir_3(void)
{

while (true) {
adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &ir3_adc);
         printf("ADC VALUE is %d\n", ir3_adc);

        vTaskDelay(100 / portTICK_PERIOD_MS);
}
}


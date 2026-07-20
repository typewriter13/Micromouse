#include <stdbool.h>
#include <stdio.h>
#include "IR_1.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"



int ir1_adc = 0;

extern adc_oneshot_unit_handle_t adc1_handle; 

void ir_1(void)
{
    

    while (true)
    {
        
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &ir1_adc);
        
        printf("ADC VALUE is %d\n", ir1_adc);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
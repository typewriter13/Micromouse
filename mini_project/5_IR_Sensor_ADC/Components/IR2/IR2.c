#include <stdbool.h>
#include <stdio.h>
#include "IR2.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "hal/adc_types.h"


int ir2_adc = 0;
extern adc_oneshot_unit_handle_t adc1_handle; 

void ir_2(void)
{
   
    while (true)
    {
        
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &ir2_adc);
        
        printf("ADC VALUE is %d\n", ir2_adc);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}



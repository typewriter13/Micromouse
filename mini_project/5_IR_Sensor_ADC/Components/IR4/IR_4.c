#include <stdbool.h>
#include <stdio.h>
#include "IR_4.h"
#include "FreeRTOSConfig.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/idf_additions.h"
#include "hal/adc_oneshot_hal.h"
#include "hal/adc_types.h"
#include "portmacro.h"


int ir4_adc =0; 
extern adc_oneshot_unit_handle_t adc1_handle; 

void ir_4(void)
{

while (true) {
adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &ir4_adc);
printf("ADC VALUE is %d\n", ir4_adc);

vTaskDelay(100 / portTICK_PERIOD_MS);
}
}

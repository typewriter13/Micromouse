#include <stdbool.h>
#include <stdio.h>
#include "IR_5.h"
#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/idf_additions.h"
#include "hal/adc_types.h"

#include "portmacro.h"


int ir5_adc=0;
extern adc_oneshot_unit_handle_t adc1_handle; 


void ir_5(void)
{

while (true) {
adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &ir5_adc);
printf("ADC Value is %d\n", ir5_adc);

vTaskDelay(100 / portTICK_PERIOD_MS);

}
}

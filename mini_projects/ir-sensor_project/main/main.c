#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ir_sensors.h"

static const char *TAG = "IR_SENSORS";

void app_main(void)
{
    ir_sensors_init();
    int readings[NUM_IR_SENSORS];

    while (1) {
        ir_sensors_read(readings);
        ESP_LOGI(TAG, "IR1:%d IR2:%d IR3:%d IR4:%d IR5:%d",
                 readings[0], readings[1], readings[2], readings[3], readings[4]);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

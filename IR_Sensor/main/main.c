#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ir_sensors.h"

void app_main(void) {
    init_ir_sensors();
    ir_sensor_reading_t reading;

    while (1) {
        read_ir_sensors(&reading);
        for (int i = 0; i < NUM_IR_SENSORS; i++) {
            printf("IR[%d]: %d\n", i, reading.readings[i]);
        }
        printf("-------------------\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

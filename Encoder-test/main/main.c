#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../src/encoder.c"

void app_main(void)
{
    encoder_init();

    while (1)
    {
        printf("Encoder count: %d\n", encoder_get_count());
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

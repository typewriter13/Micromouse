#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pindef.h" 

void app_main(void)
{
    initialize_pwm_system();

    while (1)
    {
        for (float i = 10.0; i < 80.0; i += 5.0)
        {
            pwm_motor_forward(i);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "../include/pindef.h" 

void app_main(void)
{
    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();

    while (1)
    {
        pwm_left_motor_run(50.0);
        pwm_right_motor_run(50.0);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

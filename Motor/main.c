#include <stdio.h>
#include "pwm.c"

void app_main(void)
{
    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();

    while (1)
    {
        for (float i = 10.0; i < 80.0; i += 5.0)
        {
            pwm_motor_forward(i);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

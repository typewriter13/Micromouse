#include <stdio.h>
#include "../src/pwm.c"
#include "../src/ir.c"


#define Duty_cycle 70.0

void app_main(void)
{
    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();
    ir_configuration();

    while (1)
    {
        int right_reading = 0, left_reading = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_0, &right_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_1, &left_reading));

        if (right_reading < IR_Deadband) {
            pwm_left_motor_stop();
            pwm_right_motor_run(Duty_cycle);
        } else if (left_reading < IR_Deadband) {
            pwm_right_motor_stop();
            pwm_left_motor_run(Duty_cycle);
        } else {
            pwm_left_motor_run(Duty_cycle);
            pwm_right_motor_run(Duty_cycle);
        }

        printf("Right: %d  Left: %d\n", right_reading, left_reading);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

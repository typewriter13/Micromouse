#include <stdio.h>
#include "../include/motor.h"
#include "../include/ir.h"


#define Duty_cycle 25.0
#define Max_duty_cycle 50.0
#define Min_duty_cycle 0.0

#define Kp 0.08f
#define Kd 0.05f

float pd_correction(int right_reading, int left_reading)
{
    float error = (float)(right_reading - left_reading);
    static float last_error = 0.0f;
    float derivative = error - last_error;
    last_error = error;

    float correction = Kp * error + Kd * derivative;
    return correction;
}

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

        float correction = pd_correction(right_reading, left_reading);

        printf("Right: %d  Left: %d \n", right_reading, left_reading);

        float left_motor_duty = Duty_cycle + correction;
        float right_motor_duty = Duty_cycle - correction;

        if (left_motor_duty > Max_duty_cycle)
            left_motor_duty = Max_duty_cycle;
        if (right_motor_duty > Max_duty_cycle)
            right_motor_duty = Max_duty_cycle;

        if (left_motor_duty < Min_duty_cycle)
            left_motor_duty = Min_duty_cycle;
        if (right_motor_duty < Min_duty_cycle)
            right_motor_duty = Min_duty_cycle;

        pwm_left_motor_run(left_motor_duty);
        pwm_right_motor_run(right_motor_duty);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

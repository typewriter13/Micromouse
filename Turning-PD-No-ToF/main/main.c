#include <stdio.h>
#include "../include/motor.h"
#include "../include/ir.h"


#define Duty_cycle 30.0
#define Max_duty_cycle 70.0
#define Min_duty_cycle 0.0
#define slow_duty_cycle 10.0
#define fast_duty_cycle 60.0

#define left_turn_threshold 3900
#define right_turn_threshold 3900

#define Kp 0.005f
#define Kd 0.0005f
#define dt 0.01f

float pd_correction(int right_reading, int left_reading)
{
    static float last_error = 0.0f;
    float error = (float)(right_reading - left_reading);
    printf("Error: %f\n", error);
    float derivative = (error - last_error) / dt;
    last_error = error;

    float correction = Kp * error + Kd * derivative;
    printf("Correction: %f\n", correction);
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
        int left_diag_reading = 0, right_diag_reading = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_0, &right_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_1, &left_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_2, &left_diag_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_3, &right_diag_reading));

        float left_motor_duty;
        float right_motor_duty;

        printf("Right: %d  Left: %d Left Diag: %d Right Diag: %d\n", right_reading, left_reading, left_diag_reading, right_diag_reading);

        if (left_diag_reading > left_turn_threshold) {
            left_motor_duty = slow_duty_cycle;
            right_motor_duty = fast_duty_cycle;
        } else if (right_diag_reading > right_turn_threshold) {
            right_motor_duty = slow_duty_cycle;
            left_motor_duty = fast_duty_cycle;
        } else {
        float correction = pd_correction(right_reading, left_reading);
        left_motor_duty = Duty_cycle + correction;
        right_motor_duty = Duty_cycle - correction;
        }
         
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

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

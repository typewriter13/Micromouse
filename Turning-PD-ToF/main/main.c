#include <stdio.h>
#include "../include/motor.h"
#include "../include/ir.h"
#include "../include/tof.h"


#define Duty_cycle 30.0
#define Max_duty_cycle 70.0
#define Min_duty_cycle 0.0
#define slow_duty_cycle 10.0
#define fast_duty_cycle 60.0

#define turn_threshold 3900
#define front_wall_stop_distance 40

#define Kp 0.005f
#define Kd 0.0005f
#define dt 0.01f

float pd_correction(int right_reading, int left_reading)
{
    static float last_error = 0.0f;
    float error = (float)(right_reading - left_reading);
    float derivative = (error - last_error) / dt;
    last_error = error;

    float correction = Kp * error + Kd * derivative;
    return correction;
}

void app_main(void)
{
    tof_i2c_init();

    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();
    ir_configuration();

    while (1)
    {
        int right_reading = 0, left_reading = 0;
        int left_diag_reading = 0, right_diag_reading = 0;
        float left_motor_duty = 0.0f;
        float right_motor_duty = 0.0f;
        uint16_t distance = tof_read_distance();
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_0, &right_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_1, &left_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_2, &left_diag_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_3, &right_diag_reading));

        printf("Right: %d  Left: %d  Left Diag: %d  Right Diag: %d  ToF: %d \n", right_reading, left_reading, left_diag_reading, right_diag_reading, distance);


        if (left_diag_reading > turn_threshold) {
            left_motor_duty = slow_duty_cycle;
            right_motor_duty = fast_duty_cycle;
        } else if (right_diag_reading > turn_threshold) {
            right_motor_duty = slow_duty_cycle;
            left_motor_duty = fast_duty_cycle;
        } else {
            float correction = pd_correction(right_reading, left_reading);
            left_motor_duty = Duty_cycle + correction;
            right_motor_duty = Duty_cycle - correction;
        }

        if (left_motor_duty > Max_duty_cycle) left_motor_duty = Max_duty_cycle;
        if (right_motor_duty > Max_duty_cycle) right_motor_duty = Max_duty_cycle;
        if (left_motor_duty < Min_duty_cycle) left_motor_duty = Min_duty_cycle;
        if (right_motor_duty < Min_duty_cycle) right_motor_duty = Min_duty_cycle;

        if (distance < front_wall_stop_distance) {
            pwm_motors_brake();
        } else {
            pwm_left_motor_run(left_motor_duty);
            pwm_right_motor_run(right_motor_duty);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

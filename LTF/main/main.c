#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "../include/motor.h"
#include "../include/ir.h"
#include "../include/wifi.h" 
#include "../include/tof.h"


#define Duty_cycle 16.0
#define Max_duty_cycle 70.0
#define Min_duty_cycle 0.0
#define slow_duty_cycle 16.0
#define fast_duty_cycle 23.0
#define dt 0.1f
#define TURN_DUTY         17.0f
#define LEFT_TURN_TIME_MS  225
#define RIGHT_TURN_TIME_MS 255
#define UTURN_TIME_MS      35
#define front_wall_stop_distance 42

volatile float Kp = 0.003f;
volatile float Kd = 0.0005f;
volatile int left_turn_threshold = 3900;
volatile int right_turn_threshold = 3900;
float correction;


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
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_connection();
    start_webserver();
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

        bool left_open  = (left_diag_reading  > left_turn_threshold);
        bool front_open = (distance > front_wall_stop_distance);
        bool right_open = (right_diag_reading > right_turn_threshold);

        int decision;
        if (left_open) {
            decision = 1;
        } else if (front_open) {
            decision = 2;
        } else if (right_open) {
            decision = 3;
        } else {
            decision = 4;
        }

        switch (decision)
        {
            case 1:
                turn_left();
                 correction = pd_correction(right_reading, left_reading);
                left_motor_duty  = Duty_cycle + correction;
                right_motor_duty = Duty_cycle - correction;

                if (left_motor_duty  > Max_duty_cycle) left_motor_duty  = Max_duty_cycle;
                if (right_motor_duty > Max_duty_cycle) right_motor_duty = Max_duty_cycle;
                if (left_motor_duty  < Min_duty_cycle) left_motor_duty  = Min_duty_cycle;
                if (right_motor_duty < Min_duty_cycle) right_motor_duty = Min_duty_cycle;

                pwm_left_motor_run(left_motor_duty);
                pwm_right_motor_run(right_motor_duty);
                break;
                

            case 2:
            {
                 correction = pd_correction(right_reading, left_reading);
                left_motor_duty  = Duty_cycle + correction;
                right_motor_duty = Duty_cycle - correction;

                if (left_motor_duty  > Max_duty_cycle) left_motor_duty  = Max_duty_cycle;
                if (right_motor_duty > Max_duty_cycle) right_motor_duty = Max_duty_cycle;
                if (left_motor_duty  < Min_duty_cycle) left_motor_duty  = Min_duty_cycle;
                if (right_motor_duty < Min_duty_cycle) right_motor_duty = Min_duty_cycle;

                pwm_left_motor_run(left_motor_duty);
                pwm_right_motor_run(right_motor_duty);
                break;
            }

            case 3:
                turn_right();
                 correction = pd_correction(right_reading, left_reading);
                left_motor_duty  = Duty_cycle + correction;
                right_motor_duty = Duty_cycle - correction;

                if (left_motor_duty  > Max_duty_cycle) left_motor_duty  = Max_duty_cycle;
                if (right_motor_duty > Max_duty_cycle) right_motor_duty = Max_duty_cycle;
                if (left_motor_duty  < Min_duty_cycle) left_motor_duty  = Min_duty_cycle;
                if (right_motor_duty < Min_duty_cycle) right_motor_duty = Min_duty_cycle;

                pwm_left_motor_run(left_motor_duty);
                pwm_right_motor_run(right_motor_duty);
                break;

            case 4:
            default:
                turn_around();
                 correction = pd_correction(right_reading, left_reading);
                left_motor_duty  = Duty_cycle + correction;
                right_motor_duty = Duty_cycle - correction;

                if (left_motor_duty  > Max_duty_cycle) left_motor_duty  = Max_duty_cycle;
                if (right_motor_duty > Max_duty_cycle) right_motor_duty = Max_duty_cycle;
                if (left_motor_duty  < Min_duty_cycle) left_motor_duty  = Min_duty_cycle;
                if (right_motor_duty < Min_duty_cycle) right_motor_duty = Min_duty_cycle;

                pwm_left_motor_run(left_motor_duty);
                pwm_right_motor_run(right_motor_duty);
                break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
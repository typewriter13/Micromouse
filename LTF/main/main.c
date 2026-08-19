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


#define Duty_cycle 18.0
#define Max_duty_cycle 70.0
#define Min_duty_cycle 0.0
#define slow_duty_cycle 21.0
#define fast_duty_cycle 33.0
#define dt 0.1f
#define UTURN_TIME_MS 35
#define wall_distance 42
#define turn_tof_sensor 0


volatile float Kp = 0.006f;
volatile float Kd = 0.0009f;
volatile int left_turn_threshold = 4000;
volatile int right_turn_threshold = 4000;

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

        bool left_open  = (left_diag_reading  > left_turn_threshold );
        bool front_open = (distance > wall_distance );
        bool front_close = (distance < wall_distance);
        bool right_open = (right_diag_reading > right_turn_threshold );

        int decision;
        if (left_open) {
            decision = 1;
        } else if (front_open) {
            decision = 2;
        } else if (right_open) {
            decision = 3;
        } else if (front_close) {
            decision = 4;
        }
        else {
            decision = 5;
        }

        switch (decision)
        {
            case 1:
                turn_left();
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
                break;

            case 4:
        
                turn_around();
                
                break;
            
            case 5:
            default : 
            pwm_motors_brake() ;    
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
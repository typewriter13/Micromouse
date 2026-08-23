#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "../include/motor.h"
#include "../include/ir.h"
#include "../include/wifi.h"
#include "../include/tof.h"

#define Duty_cycle 18.0
#define JUNCTION_CONFIRM_COUNT 5
#define MAIN_LOGIC_RATE 100

volatile float Kp = 0.008f;
volatile float Kd = 0.0005f;
volatile int left_turn_threshold = 3900;
volatile int right_turn_threshold = 3900;
static float last_error = 0.0f;
static int left_open_count = 0;
static int right_open_count = 0;
static uint16_t distance = 9999;

float correction;

float pd_correction(int right_reading, int left_reading, float dt)
{
    float error = (float)(right_reading - left_reading);
    float derivative = (error - last_error) / dt;
    last_error = error;

    float correction = Kp * error + Kd * derivative;
    return correction;
}

void pd_reset()
{
    last_error = 0.0f;
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
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

    uint64_t lastMicros = esp_timer_get_time();
    long timer = 0;
    long timerTof = 0;

    while (1)
    {
        uint64_t now = esp_timer_get_time();
        long elapsed = (long)(now - lastMicros);
        lastMicros = now;

        // Fixed Error 5: Decrement timers properly exactly once per loop execution
        timer -= elapsed;
        timerTof -= elapsed;

        if (timerTof <= 0)
        {
            timerTof = 1000000L / TOF_RATE;
            distance = tof_read_distance();
        }

        if (timer <= 0)
        {
            timer += 1000000L / MAIN_LOGIC_RATE;

            int right_reading = 0, left_reading = 0;
            int left_diag_reading = 0, right_diag_reading = 0;
            float left_motor_duty = 0.0f;
            float right_motor_duty = 0.0f;
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_0, &right_reading));
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_1, &left_reading));
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_2, &left_diag_reading));
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_PIN_3, &right_diag_reading));

            printf("Right: %d  Left: %d  Left Diag: %d  Right Diag: %d ToF: %d\n", right_reading, left_reading, left_diag_reading, right_diag_reading, distance);

            bool left_open = (left_diag_reading > left_turn_threshold);
            bool front_open = (distance > wall_distance);
            bool front_close = (distance < wall_distance);
            bool right_open = (right_diag_reading > right_turn_threshold);

            left_open_count = left_open ? (left_open_count + 1) : 0;
            right_open_count = right_open ? (right_open_count + 1) : 0;
            bool left_confirmed = (left_open_count >= JUNCTION_CONFIRM_COUNT);
            bool right_confirmed = (right_open_count >= JUNCTION_CONFIRM_COUNT);

            int decision;
            if (left_confirmed)
            {
                decision = 1;
            }
            else if (front_open)
            {
                decision = 2;
            }
            else if (right_confirmed)
            {
                decision = 3;
            }
            else if (front_close && !right_confirmed && !left_confirmed)
            {
                decision = 4;
            }
            else
            {
                decision = 5;
            }

            switch (decision)
            {
            case 1:
                turn_left();
                pd_reset();
                break;

            case 2:
            {
                static uint64_t last_pd_time = 0;
                uint64_t pd_now = esp_timer_get_time();

                float actual_dt = (last_pd_time > 0) ? (float)(pd_now - last_pd_time) / 1000000.0f : 0.01f;
                last_pd_time = pd_now;
                correction = pd_correction(right_reading, left_reading, actual_dt);
                left_motor_duty = Duty_cycle + correction;
                right_motor_duty = Duty_cycle - correction;

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
                break;
            }

            case 3:
                turn_right();
                pd_reset();
                break;

            case 4:

                turn_around();
                pd_reset();
                break;

            case 5:
            default:
                pwm_motors_brake();
            }
        }
        // static unsigned long lastMicrosTof = esp_timer_get_time();
        // unsigned long nowMs = millis();

        // unsigned long nowTof = esp_timer_get_time(), dtTof = nowTof - lastMicrosTof;
        // lastMicrosTof = nowTof;
        // static long timerTof = 0;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

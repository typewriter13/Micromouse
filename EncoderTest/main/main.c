#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/pulse_cnt.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "ENC_TEST";

// ---------- DRV8833 pins ----------
#define MOTOR_AIN1_GPIO     15
#define MOTOR_AIN2_GPIO     16
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE            LEDC_LOW_SPEED_MODE
#define LEDC_FREQ_HZ         20000
#define LEDC_RES             LEDC_TIMER_10_BIT
#define LEDC_CH_A            LEDC_CHANNEL_0
#define LEDC_CH_B            LEDC_CHANNEL_1

// ---------- Encoder pins ----------
#define ENCODER_A_GPIO      4
#define ENCODER_B_GPIO      5
#define PCNT_HIGH_LIMIT      32767
#define PCNT_LOW_LIMIT      -32768

// ---------- Encoder scaling ----------
// EDIT THESE based on your N20 encoder's datasheet:
#define MOTOR_PPR            7        // pulses per rev of MOTOR shaft (check datasheet, common values: 7, 11)
#define GEAR_RATIO           10.0f   // your N20's gearbox ratio, e.g. 100:1 (check the printed code on the motor body)
#define X_DECODE             4        // x4 decoding (rising+falling edges on both channels)

// Total counts for one full revolution of the OUTPUT shaft:
#define COUNTS_PER_OUTPUT_REV   (MOTOR_PPR * X_DECODE * GEAR_RATIO)

static pcnt_unit_handle_t pcnt_unit = NULL;

// ---------------- Motor driver setup ----------------
static void motor_pwm_init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t ch_a = {
        .gpio_num   = MOTOR_AIN1_GPIO,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CH_A,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_a));

    ledc_channel_config_t ch_b = {
        .gpio_num   = MOTOR_AIN2_GPIO,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CH_B,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_b));
}

static void motor_forward(int speed)
{
    ledc_set_duty(LEDC_MODE, LEDC_CH_A, speed);
    ledc_update_duty(LEDC_MODE, LEDC_CH_A);
    ledc_set_duty(LEDC_MODE, LEDC_CH_B, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CH_B);
}

static void motor_reverse(int speed)
{
    ledc_set_duty(LEDC_MODE, LEDC_CH_A, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CH_A);
    ledc_set_duty(LEDC_MODE, LEDC_CH_B, speed);
    ledc_update_duty(LEDC_MODE, LEDC_CH_B);
}

static void motor_stop(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CH_A, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CH_A);
    ledc_set_duty(LEDC_MODE, LEDC_CH_B, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CH_B);
}

// ---------------- Encoder (PCNT) setup ----------------
static void encoder_init(void)
{
    pcnt_unit_config_t unit_config = {
        .high_limit = PCNT_HIGH_LIMIT,
        .low_limit  = PCNT_LOW_LIMIT,
        .flags.accum_count = true,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ENCODER_A_GPIO,
        .level_gpio_num = ENCODER_B_GPIO,
    };
    pcnt_channel_handle_t chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num  = ENCODER_B_GPIO,
        .level_gpio_num = ENCODER_A_GPIO,
    };
    pcnt_channel_handle_t chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_LOW_LIMIT));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
}

// Converts raw count to degrees (handles multi-revolution, unwrapped angle)
static float count_to_degrees(int count)
{
    return ((float)count / COUNTS_PER_OUTPUT_REV) * 360.0f;
}

// Prints encoder angle every 100ms for `duration_ms`
static void print_encoder_for(int duration_ms)
{
    int elapsed = 0;
    while (elapsed < duration_ms) {
        int count = 0;
        ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &count));

        float degrees = count_to_degrees(count);
        float wrapped = fmodf(degrees, 360.0f);   // 0-360 wrapped angle
        if (wrapped < 0) wrapped += 360.0f;

        ESP_LOGI(TAG, "Count: %d | Angle: %.2f deg | Wrapped: %.2f deg",
                 count, degrees, wrapped);

        vTaskDelay(pdMS_TO_TICKS(100));
        elapsed += 100;
    }
}

void app_main(void)
{
    motor_pwm_init();
    encoder_init();

    int speed = 1020;
    int run_time_ms = 10000;

    while (1) {
        ESP_LOGI(TAG, "---- FORWARD ----");
        motor_forward(speed);
        print_encoder_for(run_time_ms);

        motor_stop();
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGI(TAG, "---- REVERSE ----");
        motor_reverse(speed);
        print_encoder_for(run_time_ms);

        motor_stop();
        vTaskDelay(pdMS_TO_TICKS(1000));

        break;
    }
}
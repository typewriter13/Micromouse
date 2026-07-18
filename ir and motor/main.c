#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "sdkconfig.h"

// --- Motor Pin Definitions ---
#define MOTOR_PIN_0A 4
#define MOTOR_PIN_0B 5
#define MOTOR_PIN_1A 6
#define MOTOR_PIN_1B 7

// --- ADC Definitions ---
#define ADC_PIN       ADC_CHANNEL_7     // Channel 7 - Check ESP32 Pinout for the GPIO Number
#define ADC_UNIT      ADC_UNIT_1        // ADC1
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

// --- Motor PWM Functions ---
void initialize_pwm_gpio_pins()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_PIN_0A);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOTOR_PIN_0B);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, MOTOR_PIN_1A);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, MOTOR_PIN_1B);
}

void pwm_gpio_configuration()
{
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
}

void pwm_motor_forward(float duty_cycle)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}

void pwm_motor_backward(float duty_cycle)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}

// --- ADC Functions ---
adc_oneshot_unit_handle_t adc_handle;

void initialize_adc()
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN, &config));
}

int read_adc()
{
    int adc_value;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
    return adc_value;
}

// --- Main Application Entry Point ---
void app_main(void)
{
    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();
    initialize_adc();

    while (1)
    {
        for (float i = 10.0; i < 80.0; i += 5.0)
        {
            pwm_motor_forward(i);
            int adc_value = read_adc();
            ESP_LOGI("ADC Value", "%d", adc_value);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        for (float i = 10.0; i < 80.0; i += 5.0)
        {
            pwm_motor_backward(i);
            int adc_value = read_adc();
            ESP_LOGI("ADC Value", "%d", adc_value);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}
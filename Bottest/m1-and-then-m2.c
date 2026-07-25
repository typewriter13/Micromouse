#include <stdio.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"

static const char *TAG = "MICROMOUSE";

//I2C / ToF Definition
#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SDA_IO      41
#define I2C_MASTER_SCL_IO      42
#define I2C_MASTER_FREQ_HZ     100000

#define VL53L0X_ADDR           0x29
#define SYSRANGE_START_REG     0x00
#define RESULT_RANGE_REG       0x1E

#define LOWER_THRESHOLD        20
#define UPPER_THRESHOLD        2000

//Motor Pin Definitions
#define MOTOR_PIN_0A 15
#define MOTOR_PIN_0B 16
#define MOTOR_PIN_1A 17
#define MOTOR_PIN_1B 18

#define MOTOR_DUTY_CYCLE       50.0
#define TEST_RUN_TIME_MS       3000

//ADC Definitions (2 IR sensors)
#define ADC_PIN_1     ADC_CHANNEL_8
#define ADC_PIN_2     ADC_CHANNEL_9
#define ADC_UNIT      ADC_UNIT_1
#define ADC_BITWIDTH  ADC_BITWIDTH_12
#define ADC_ATTEN     ADC_ATTEN_DB_12

adc_oneshot_unit_handle_t adc_handle;

//ToF

esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

void tof_i2c_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGI(TAG, "I2C initialized for ToF sensor.");
}

//Motor PWM

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

void stop_all_motors(void)
{
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 0);
}

// Runs ONLY the left motor (TIMER_0). Right motor stays at 0 duty.
void run_left_motor_only(float duty_cycle)
{
    stop_all_motors();
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    ESP_LOGI(TAG, "LEFT motor only -> Duty: %.1f%%", duty_cycle);
}

// Runs ONLY the right motor (TIMER_1). Left motor stays at 0 duty.
void run_right_motor_only(float duty_cycle)
{
    stop_all_motors();
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    ESP_LOGI(TAG, "RIGHT motor only -> Duty: %.1f%%", duty_cycle);
}

//IR Sensors

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
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN_1, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN_2, &config));
}

int read_adc(adc_channel_t channel)
{
    int adc_value;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &adc_value));
    return adc_value;
}

void i2c_scan(void)
{
    ESP_LOGI(TAG, "Starting I2C bus scan...");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found device at address 0x%02X", addr);
            found++;
        }
    }
    if (found == 0) {
        ESP_LOGW(TAG, "No I2C devices found on the bus.");
    } else {
        ESP_LOGI(TAG, "Scan complete. %d device(s) found.", found);
    }
}

void app_main(void)
{
    tof_i2c_init();
	i2c_scan();
    initialize_adc();
    initialize_pwm_gpio_pins();
    pwm_gpio_configuration();

    uint8_t model_id;
    if (i2c_read_reg(VL53L0X_ADDR, 0xC0, &model_id, 1) != ESP_OK) {
        ESP_LOGE(TAG, "ToF Sensor failed to respond at address 0x29! Check I2C wiring.");
    } else {
        ESP_LOGI(TAG, "ToF Sensor detected successfully.");
    }

    while (1)
    {
        run_left_motor_only(MOTOR_DUTY_CYCLE);
        vTaskDelay(TEST_RUN_TIME_MS / portTICK_PERIOD_MS);
        stop_all_motors();
        ESP_LOGI(TAG, "Left motor test done. Stopping before right motor test.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        run_right_motor_only(MOTOR_DUTY_CYCLE);
        vTaskDelay(TEST_RUN_TIME_MS / portTICK_PERIOD_MS);
        stop_all_motors();
        ESP_LOGI(TAG, "Right motor test done. Repeating cycle.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

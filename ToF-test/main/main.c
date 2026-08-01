#include <stdio.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 41
#define I2C_MASTER_SCL_IO 42
#define I2C_MASTER_FREQ_HZ 100000

#define VL53L0X_DEFAULT_ADDR 0x29
#define I2C_SLAVE_DEVICE_ADDRESS_REG 0x8A
#define SYSRANGE_START_REG 0x00
#define RESULT_RANGE_REG 0x1E

#define LOWER_THRESHOLD 20
#define UPPER_THRESHOLD 2000

#define TOF_SENSOR_COUNT 5

static const char *TAG = "VL53L0X";

// One dedicated XSHUT GPIO per sensor
static const int xshut_pins[TOF_SENSOR_COUNT] = {4, 5, 6, 15, 16};

// Unique addresses we reassign each sensor to during startup
static const uint8_t tof_addresses[TOF_SENSOR_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34};

esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

void tof_i2c_init(void)
{
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
}

void tof_xshut_init(void)
{
    // Hold ALL sensors in shutdown first, so none respond on the bus yet
    for (int i = 0; i < TOF_SENSOR_COUNT; i++) {
        gpio_set_direction(xshut_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(xshut_pins[i], 0);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

esp_err_t tof_assign_addresses(void)
{
    for (int i = 0; i < TOF_SENSOR_COUNT; i++) {
        // Power up ONLY this sensor - it's the only one that will answer to 0x29
        gpio_set_level(xshut_pins[i], 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        uint8_t model_id;
        if (i2c_read_reg(VL53L0X_DEFAULT_ADDR, 0xC0, &model_id, 1) != ESP_OK) {
            ESP_LOGE(TAG, "Sensor %d not responding at default address. Check wiring!", i);
            return ESP_FAIL;
        }

        esp_err_t err = i2c_write_reg(VL53L0X_DEFAULT_ADDR, I2C_SLAVE_DEVICE_ADDRESS_REG, tof_addresses[i] & 0x7F);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reassign address for sensor %d", i);
            return err;
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Sensor %d assigned address 0x%02X", i, tof_addresses[i]);
    }
    return ESP_OK;
}

void tof_read_all(uint16_t distances_mm[TOF_SENSOR_COUNT])
{
    // Trigger a ranging measurement on all 5 sensors
    for (int i = 0; i < TOF_SENSOR_COUNT; i++) {
        i2c_write_reg(tof_addresses[i], SYSRANGE_START_REG, 0x01);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);  // give them time to finish ranging

    // Then read each one's result
    for (int i = 0; i < TOF_SENSOR_COUNT; i++) {
        uint8_t dist_data[2];
        if (i2c_read_reg(tof_addresses[i], RESULT_RANGE_REG, dist_data, 2) == ESP_OK) {
            distances_mm[i] = ((dist_data[0] << 8) | dist_data[1]) / 2;
        } else {
            distances_mm[i] = 0;
        }
    }
}

void app_main() {
    tof_i2c_init();
    ESP_LOGI(TAG, "I2C hardware initialized.");

    tof_xshut_init();

    if (tof_assign_addresses() != ESP_OK) {
        ESP_LOGE(TAG, "Sensor address assignment failed. Check XSHUT wiring!");
        return;
    }

    uint16_t distances[TOF_SENSOR_COUNT];

    while (1) {
        tof_read_all(distances);

        for (int i = 0; i < TOF_SENSOR_COUNT; i++) {
            if (distances[i] > LOWER_THRESHOLD && distances[i] < UPPER_THRESHOLD) {
                ESP_LOGI(TAG, "Sensor %d: %d mm", i, distances[i]);
            } else {
                ESP_LOGW(TAG, "Sensor %d: Out of range", i);
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
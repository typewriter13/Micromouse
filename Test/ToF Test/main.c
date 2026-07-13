#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 41
#define I2C_MASTER_SCL_IO 42
#define I2C_MASTER_FREQ_HZ 100000
#define VL53L0X_ADDR 0x29
#define LOWER_THRESHOLD 20
#define UPPER_THRESHOLD 2000

static const char *TAG = "VL53L0X";

esp_err_t i2c_write_reg(uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, VL53L0X_ADDR, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

esp_err_t i2c_read_reg(uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, VL53L0X_ADDR, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

void vl530X_config(void)
{
	 i2c_config_t i2c_config = {
	        .mode = I2C_MODE_MASTER,
	        .sda_io_num = I2C_MASTER_SDA_IO,
	        .scl_io_num = I2C_MASTER_SCL_IO,
	        .sda_pullup_en = GPIO_PULLUP_ENABLE,
	        .scl_pullup_en = GPIO_PULLUP_ENABLE,
	        .master.clk_speed = I2C_MASTER_FREQ_HZ,
	    };
	 ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM,  &i2c_config));
	 ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0));
}

void app_main() {
    vl530X_config();


    ESP_LOGI(TAG, "I2C hardware initialized.");

    uint8_t model_id;
    if (i2c_read_reg(0xC0, &model_id, 1) == ESP_OK) {
        ESP_LOGI(TAG, "Sensor Model ID: 0x%02X (Expected: 0xEE)", model_id);
    } else {
        ESP_LOGE(TAG, "Failed to communicate with sensor. Check wiring!");
        return;
    }
    while (1) {
        i2c_write_reg(0x00, 0x01);

        vTaskDelay(50 / portTICK_PERIOD_MS);

        uint8_t dist_data[2];
        if (i2c_read_reg(0x1E, dist_data, 2) == ESP_OK) {
            uint16_t distance_mm = (dist_data[0] << 8) | dist_data[1];

          if (distance_mm > LOWER_THRESHOLD  && distance_mm <UPPER_THRESHOLD ) {
            ESP_LOGI(TAG, "Distance: %d mm", distance_mm);
            } else {
            	ESP_LOGW(TAG, "Distance: Out of range");
            }
            }

    else {
            ESP_LOGE(TAG, "Failed to read distance data from I2C bus.");
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

